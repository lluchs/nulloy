/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2014 Sergey Vlasov <sergey@vlasov.me>
**
**  This program can be distributed under the terms of the GNU
**  General Public License version 3.0 as published by the Free
**  Software Foundation and appearing in the file LICENSE.GPL3
**  included in the packaging of this file.  Please review the
**  following information to ensure the GNU General Public License
**  version 3.0 requirements will be met:
**
**  http://www.gnu.org/licenses/gpl-3.0.html
**
*********************************************************************/

#include "waveformSlider.h"
#include "waveformBuilderInterface.h"
#include "pluginLoader.h"
#include "settings.h"
#include "common.h"

#include <QMouseEvent>
#include <QFile>
#include <QStylePainter>
#include <QStyleOptionFocusRect>

NWaveformSlider::NWaveformSlider(QWidget *parent) : QAbstractSlider(parent)
{
	m_radius = 0;
	m_background = QBrush(Qt::darkBlue);
	m_waveBackground = QBrush(Qt::darkGreen);
	m_waveBorderColor = QColor(Qt::green);
	m_progressPlayingBackground = QBrush(Qt::darkCyan);
	m_progressPausedBackground = QBrush(Qt::darkGray);
	m_playingComposition = QPainter::CompositionMode_Overlay;
	m_pausedComposition = QPainter::CompositionMode_Overlay;

	setMinimum(0);
	setMaximum(10000);

	m_waveBuilder = dynamic_cast<NWaveformBuilderInterface *>(NPluginLoader::getPlugin(N::WaveformBuilder));

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(checkForUpdate()));
	m_timer->start();

	setAcceptDrops(TRUE);

	m_oldSize = QSize(0, 0);
	init();
}

void NWaveformSlider::setPausedState(bool state)
{
	m_pausedState = state;
	update();
}

void NWaveformSlider::init()
{
	m_oldIndex = -1;
	m_oldBuildPos = -1;
	m_pausedState = FALSE;
	m_needsUpdate = FALSE;
	setEnabled(FALSE);

	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	setMinimumHeight(50);
	setMinimumWidth(150);
}

QSize NWaveformSlider::sizeHint() const
{
	return QSize(200, 80);
}

NWaveformSlider::~NWaveformSlider() {}

void NWaveformSlider::checkForUpdate()
{
	if (!m_waveBuilder)
		return;

	float pos;
	int index;
	m_waveBuilder->positionAndIndex(pos, index);

	if (m_oldSize != size() || m_oldIndex != index)
		m_needsUpdate = TRUE;

	if (m_needsUpdate) {
		QPainter painter;
		QImage waveImage = m_normalImage = m_playingImage = m_pausedImage = QImage(size(), QImage::Format_ARGB32_Premultiplied);
		waveImage.fill(0);

		m_oldBuildPos = pos;
		m_oldIndex = index;
		m_oldSize = size();


		// waveform >>
		painter.begin(&waveImage);
		painter.setBrush(m_waveBackground);
		QPen wavePen;
		wavePen.setWidth(0);
		wavePen.setColor(m_waveBorderColor);
		painter.setPen(wavePen);

		painter.translate(1, 1);
		painter.scale((qreal)(width() - 1) / m_oldIndex * m_oldBuildPos, (height() - 2) / 2);
		painter.translate(0, 1);

		QPainterPath pathPos;
		QPainterPath pathNeg;
		NWaveformPeaks *peaks = m_waveBuilder->peaks();
		for (int i = 0; i < m_oldIndex; ++i) {
			pathPos.lineTo(i, peaks->positive(i));
			pathNeg.lineTo(i, peaks->negative(i));
		}
		QPainterPath fullPath(pathNeg);
		fullPath.connectPath(pathPos.toReversed());
		fullPath.closeSubpath();
		painter.setRenderHint(QPainter::Antialiasing);
		painter.drawPath(fullPath);
		painter.end();
		// << waveform


		QList<QImage *> images; images << &m_normalImage << &m_playingImage << &m_pausedImage;
		QList<QPainter::CompositionMode> modes; modes << QPainter::CompositionMode_SourceOver << m_playingComposition << m_pausedComposition;
		QList<QBrush> brushes; brushes << m_background << m_progressPlayingBackground << m_progressPausedBackground;
		for (int i = 0; i < images.size(); ++i) {
			painter.begin(images[i]);
			// background
			painter.setPen(Qt::NoPen);
			painter.setBrush(brushes[i]);
			painter.setRenderHint(QPainter::Antialiasing);
			painter.drawRoundedRect(rect(), m_radius, m_radius);
			// background + waveform
			painter.setCompositionMode(modes[i]);
			painter.drawImage(0, 0, waveImage);
			painter.end();
		}

		update();
		m_needsUpdate = FALSE;
	}
}

void NWaveformSlider::paintEvent(QPaintEvent *)
{
	QPainter painter(this);

	if (isEnabled()) {
		int x = qRound((qreal)value() / maximum() * width());

		QRect right = rect().adjusted(x, 0, 0, 0);
		painter.drawImage(right, m_normalImage, right);

		QRect left = rect().adjusted(0, 0, x - width(), 0);
		painter.drawImage(left, m_pausedState ? m_pausedImage : m_playingImage, left);
	} else {
		painter.setPen(Qt::NoPen);
		painter.setBrush(m_background);
		painter.drawRoundedRect(rect(), m_radius, m_radius);
	}
}

void NWaveformSlider::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
		return;

	qreal value = (qreal)event->x() / width();

	emit sliderMoved(value);
	setValue(value);
}

void NWaveformSlider::wheelEvent(QWheelEvent *event)
{
	event->ignore();
}

void NWaveformSlider::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::StyleChange)
		m_needsUpdate = TRUE;
	QWidget::changeEvent(event);
}

void NWaveformSlider::setValue(qreal value)
{
	QAbstractSlider::setValue(value * maximum());
}

void NWaveformSlider::drawFile(const QString &file)
{
	init();

	if (!QFile(file).exists())
		return;

	m_waveBuilder->start(file);
	setEnabled(TRUE);
}

// STYLESHEET PROPERTIES >>
int NWaveformSlider::radius()
{
	return m_radius;
}

void NWaveformSlider::setRadius(int radius)
{
	m_radius = radius;
}

QBrush NWaveformSlider::background()
{
	return m_background;
}

void NWaveformSlider::setBackground(QBrush brush)
{
	m_background = brush;
}

QBrush NWaveformSlider::waveBackground()
{
	return m_waveBackground;
}

void NWaveformSlider::setWaveBackground(QBrush brush)
{
	m_waveBackground = brush;
}

QColor NWaveformSlider::waveBorderColor()
{
	return m_waveBorderColor;
}

void NWaveformSlider::setWaveBorderColor(QColor color)
{
	m_waveBorderColor = color;
}

QBrush NWaveformSlider::progressPlayingBackground()
{
	return m_progressPlayingBackground;
}

void NWaveformSlider::setProgressPlayingBackground(QBrush brush)
{
	m_progressPlayingBackground = brush;
}

QBrush NWaveformSlider::progressPausedBackground()
{
	return m_progressPausedBackground;
}

void NWaveformSlider::setProgressPausedBackground(QBrush brush)
{
	m_progressPausedBackground = brush;
}

QString NWaveformSlider::playingComposition()
{
	return ENUM_TO_STR(N, CompositionMode, m_playingComposition);
}

void NWaveformSlider::setPlayingComposition(const QString &mode)
{
	m_playingComposition = (QPainter::CompositionMode)STR_TO_ENUM(N, CompositionMode, mode.toAscii());
}

QString NWaveformSlider::pausedComposition()
{
	return ENUM_TO_STR(N, CompositionMode, m_pausedComposition);
}

void NWaveformSlider::setPausedComposition(const QString &mode)
{
	m_pausedComposition = (QPainter::CompositionMode)STR_TO_ENUM(N, CompositionMode, mode.toAscii());
}
// << STYLESHEET PROPERTIES


// DRAG & DROP >>
void NWaveformSlider::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData() && event->mimeData()->hasUrls() && !event->mimeData()->urls().isEmpty())
		event->acceptProposedAction();
	else
		event->ignore();
}

void NWaveformSlider::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

void NWaveformSlider::dragLeaveEvent(QDragLeaveEvent *event)
{
	event->accept();
}

void NWaveformSlider::dropEvent(QDropEvent *event)
{
	const QMimeData *data = event->mimeData();
	if (data->hasUrls()) {
		QStringList files;
		foreach (QUrl url, data->urls())
			files << NCore::dirListRecursive(url.toLocalFile(), NSettings::instance()->value("FileFilters").toStringList());
		emit filesDropped(files);
	}

	event->acceptProposedAction();
}

QStringList NWaveformSlider::mimeTypes() const
{
	QStringList qstrList;
	qstrList.append("text/uri-list");
	return qstrList;
}
// << DRAG & DROP
