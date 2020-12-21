/*
MIT License

Copyright (c) 2020 Maifee Ul Asad

Copyright (c) 2012 EasyPaint https://github.com/Gr1N/EasyPaint


Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

GitHub repo : https://github.com/maifeeulasad/Paint

A copy of the License : https://github.com/maifeeulasad/Paint/blob/main/LICENSE
*/

#include "toolbar.h"
#include "colorchooser.h"
#include "../datasingleton.h"

#include <QToolButton>
#include <QGridLayout>
#include <QSpinBox>
#include <QAction>
#include <QtCore/QMap>
#include <QLabel>

ToolBar::ToolBar(const QMap<InstrumentsEnum, QAction *> &actMap, QWidget *parent) :
    QToolBar(tr("Instruments"), parent), mActMap(actMap)
{
    setMovable(false);
    initializeItems();
    mPrevInstrumentSetted = false;
}

QToolButton* ToolBar::createToolButton(QAction *act)
{
    QToolButton *toolButton = new QToolButton();
    toolButton->setMinimumSize(QSize(30, 30));
    toolButton->setMaximumSize(QSize(30, 30));
    toolButton->setDefaultAction(act);
    toolButton->setStatusTip(act->text());
    return toolButton;
}

void ToolBar::initializeItems()
{
    QLabel *InstrumentText = new QLabel(tr("Instruments"), this);
    mCursorButton = createToolButton(mActMap[CURSOR]);
    mEraserButton = createToolButton(mActMap[ERASER]);
    mPenButton = createToolButton(mActMap[PEN]);
    mLineButton = createToolButton(mActMap[LINE]);
    mColorPickerPaletteButton = createToolButton(mActMap[COLORPICKERPALETTE]);
    mMagnifierButton = createToolButton(mActMap[MAGNIFIER]);
    mSprayButton = createToolButton(mActMap[SPRAY]);
    mFillButton = createToolButton(mActMap[FILL]);
    mRectangleButton = createToolButton(mActMap[RECTANGLE]);
    mEllipseButton = createToolButton(mActMap[ELLIPSE]);
    mCurveButton = createToolButton(mActMap[CURVELINE]);
    mTextButton = createToolButton(mActMap[TEXT]);
    mCropButton = createToolButton(mActMap[CROP]);

    textPenSize = new QLabel(tr("Pen size: 1 "), this);
    mPenSize = new QSlider(Qt::Horizontal);
    mPenSize->setMinimum(1);
    mPenSize->setMaximum(25);
    mPenSize->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    connect(mPenSize, &QAbstractSlider::valueChanged, this, &ToolBar::valuePenSize);

    QGridLayout *bLayout = new QGridLayout();
    bLayout->setMargin(1);
    bLayout->addWidget(InstrumentText, 0, 0, 1, 4, Qt::AlignCenter);
    bLayout->addWidget(mPenButton, 1, 0);
    bLayout->addWidget(mSprayButton, 1, 1);
    bLayout->addWidget(mEraserButton, 1, 2);
    bLayout->addWidget(mLineButton, 1, 3);
    bLayout->addWidget(mCurveButton, 2, 0);
    bLayout->addWidget(mRectangleButton, 2, 1);
    bLayout->addWidget(mEllipseButton, 2, 2);
    bLayout->addWidget(mCropButton, 2, 3);
    bLayout->addWidget(mCursorButton, 3, 0);
    bLayout->addWidget(mMagnifierButton, 3, 1);
    bLayout->addWidget(mTextButton, 3, 2);
    bLayout->addWidget(textPenSize, 4, 0, 1, 4);
    bLayout->addWidget(mPenSize, 5, 0, 1, 4);


    QWidget *bWidget = new QWidget();
    bWidget->setLayout(bLayout);

    QLabel *ColorText = new QLabel(tr("Color"), this);

    mPColorChooser = new ColorChooser(0, 0, 0, this);
    mPColorChooser->setStatusTip(tr("Primary color"));
    mPColorChooser->setToolTip(tr("Primary color"));
    connect(mPColorChooser, &ColorChooser::sendColor, this, &ToolBar::primaryColorChanged);

    mSColorChooser = new ColorChooser(255, 255, 255, this);
    mSColorChooser->setStatusTip(tr("Secondary color"));
    mSColorChooser->setToolTip(tr("Secondary color"));
    connect(mSColorChooser, &ColorChooser::sendColor, this, &ToolBar::secondaryColorChanged);

    QGridLayout *tLayout = new QGridLayout();
    tLayout->setMargin(1);
    tLayout->addWidget(ColorText, 0, 0, 1, 4, Qt::AlignCenter);
    tLayout->addWidget(mColorPickerPaletteButton, 1, 0);
    tLayout->addWidget(mFillButton, 1, 1);
    tLayout->addWidget(mPColorChooser, 1, 2);
    tLayout->addWidget(mSColorChooser, 1, 3);

    QWidget *tWidget = new QWidget();
    tWidget->setLayout(tLayout);

    addWidget(bWidget);
    addSeparator();
    addWidget(tWidget);
}

void ToolBar::valuePenSize(const int &value)
{
  DataSingleton::Instance()->setPenSize(value);
  textPenSize->setText(QString(tr("Pen size: %1")).arg(mPenSize->value()));
}

void ToolBar::primaryColorChanged(const QColor &color)
{
    DataSingleton::Instance()->setPrimaryColor(color);
}

void ToolBar::secondaryColorChanged(const QColor &color)
{
    DataSingleton::Instance()->setSecondaryColor(color);
}

void ToolBar::setPrimaryColorView()
{
    mPColorChooser->setColor(DataSingleton::Instance()->getPrimaryColor());
}

void ToolBar::setSecondaryColorView()
{
    mSColorChooser->setColor(DataSingleton::Instance()->getSecondaryColor());
}

void ToolBar::contextMenuEvent(QContextMenuEvent *)
{
}
