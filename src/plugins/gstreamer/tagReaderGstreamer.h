/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2015 Sergey Vlasov <sergey@vlasov.me>
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

#ifndef N_TAG_READER_GSTREAMER_H
#define N_TAG_READER_GSTREAMER_H

#include "global.h"
#include "plugin.h"
#include "tagReaderInterface.h"
#include <gst/gst.h>

class NTagReaderGstreamer : public NTagReaderInterface, public NPlugin
{
	Q_OBJECT
	Q_INTERFACES(NTagReaderInterface NPlugin)

private:
	QString m_path;
	GstTagList *m_taglist;
	gint64 m_nanosecs;
	float m_sampleRate;
	int m_bitDepth;
	bool m_isValid;
	QString m_codecName;
	QString parse(const QString &format, bool *success, const QString &encoding, bool stopOnFail = false);

public:
	NTagReaderGstreamer(QObject *parent = NULL) : NTagReaderInterface(parent) {}
	~NTagReaderGstreamer();

	void init();
	QString interfaceString() { return NTagReaderInterface::interfaceString(); }
	N::PluginType type() { return N::TagReader; }

	void setSource(const QString &file);
	QString toString(const QString &format, const QString &encoding);
	bool isValid();
};

#endif

