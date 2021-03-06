//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef _qtOverlay_h
#define _qtOverlay_h

#include <QWidget>
#include <QPointer>
#include "smtk/extension/qt/QtSMTKExports.h"

namespace smtk
{
  namespace attribute
  {
    class QTSMTK_EXPORT  qtOverlay : public QWidget
    {
      Q_OBJECT

    public:
        qtOverlay(QWidget * parent = 0);
        void addOverlayWidget(QWidget*w);
        void setColor(const QColor& ocolor)
        { this->m_overlayColor = ocolor; }

    protected:
        void paintEvent(QPaintEvent *);
        QPointer<QWidget> m_overlayWidget;
        QColor m_overlayColor;
    };


 //----------------------------------------------------------------------------
    class QTSMTK_EXPORT  qtOverlayFilter : public QObject
    {
      Q_OBJECT

    public:
       // onWidget is what the overlay will be covering
        qtOverlayFilter(QWidget* onWidget, QObject * parent = 0);
        virtual ~qtOverlayFilter();
        qtOverlay* overlay() { return m_overlay; }
        void setActive(bool val);
        bool active(){return m_Active;}
        void addOverlayWidget(QWidget*w);

    protected:
        bool eventFilter(QObject * obj, QEvent * ev);

        QPointer<qtOverlay> m_overlay;
        QPointer<QWidget> m_overlayOn;
//        QPointer<QWidget> m_overlayWidget;
        bool m_Active;
    };
  };
};

#endif // !_qtOverlay_
