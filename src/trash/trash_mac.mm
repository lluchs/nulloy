/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2016 Sergey Vlasov <sergey@vlasov.me>
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

#include <QString>
#import "Cocoa/Cocoa.h"
#import "Foundation/Foundation.h"

static inline NSString* fromQString(const QString &string)
{
    char *cString = string.toUtf8().data();
    return [[NSString alloc] initWithUTF8String:cString];
}

int _trash(const QString &file, QString *error)
{
    /*NSMutableArray *urls = [[NSMutableArray alloc] init];
    NSString *string = fromQString(file);
    NSLog(@"%@", string);
    [urls addObject:[NSURL fileURLWithPath:string]];
    [[NSWorkspace sharedWorkspace] recycleURLs:urls comletionHandlerL:nil];*/

    /*[[NSWorkspace sharedWorkspace] performFileOperation:NSWorkspaceRecycleOperation
                                                   source:@"/Users/admin/Desktop/"
                                              destination:@""
                           files:[NSArray arrayWithObject:@"replacepath2.py"]
                                                      tag:nil];*/

    FSRef fsRef;
    NSString *file_ns = fromQString(file);
    FSPathMakeRefWithOptions((const UInt8 *)[file_ns fileSystemRepresentation],
                             kFSPathMakeRefDoNotFollowLeafSymlink, &fsRef, NULL);
    return FSMoveObjectToTrashSync(&fsRef, NULL, kFSFileOperationDefaultOptions);;
}

