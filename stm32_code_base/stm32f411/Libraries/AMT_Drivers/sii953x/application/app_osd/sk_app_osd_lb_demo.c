//***************************************************************************
//!file     sk_app_osd_lb_demo.c
//!brief    OSD List Box demo
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include "string.h"

#include "sk_application.h"
#include "sk_app_osd.h"

//-------------------------------------------------------------------------------------------------
// Local and global data and variables
//-------------------------------------------------------------------------------------------------

int  winRefList      = -1;
static char itemList1 [ LISTBOX1_ITEM_MAX * LISTBOX1_ITEM_LENGTH];  // LISTBOX1_ITEM_LENGTH includes terminating null

//-------------------------------------------------------------------------------------------------
//! @brief      List box demo user window function.
//! @param[in]  winRef      Return value from the SiiOsdCreateListBox function.
//! @param[in]  action
//! @param[in]  pArgs
//! @return
//-------------------------------------------------------------------------------------------------
bool_t UserFuncListBoxDemo ( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t  result = true;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
        case UFA_CREATE:
            if ( SiiOsdAttachExternalData( winRef, (void *)&itemList1 ))
            {
                SiiOsdSetWindowPrivateData( winRef, LISTBOX1_ITEM_LENGTH, LISTBOX1_ITEM_MAX, 0, 0, 0 );    // IForm doesn't let me specify these yet
                SiiOsdListBoxAddItem( winRef, "The First Item", 0, 0 );
                SiiOsdListBoxAddItem( winRef, "Second row", -1, 0 );
                SiiOsdListBoxAddItem( winRef, "Row three", -1, 0 );
                SiiOsdListBoxAddItem( winRef, "fourth row here", -1, 0 );
                SiiOsdListBoxAddItem( winRef, "Item # five", -1, 0 );
                SiiOsdListBoxAddItem( winRef, "Very long item string here!!!", -1, 0 );
                SiiOsdListBoxAddItem( winRef, "Seventh item", -1, 0 );
                SiiOsdListBoxAddItem( winRef, "Row Ocho", -1, 0 );
                result = true;
            }
            else
            {
                result = false;     // Replace or Create failed
            }
            break;
        case UFA_ROW_ENTER:
            result = false;
            break;

        case UFA_FOCUS:
            break;
        case UFA_BACK:
            break;

        case UFA_SELECT:
        case UFA_CANCEL:
        case UFA_CLOSE:

            // Save the selected item and restore the demo menu
            SiiOsdListBoxGetItem( winRef, SiiOsdListBoxGetSelect( winRef), osdApp.itemString, sizeof(osdApp.itemString) );
            SkAppOsdRestoreOsdDemoMenu( winRef );
            result = false;     // Don't close the window
            break;
        default:
            break;
    }

    return( result );
}

