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

#include "colorpickerpaletteinstrument.h"
#include "../imagearea.h"
#include "../datasingleton.h"
#include <QColorDialog>

ColorpickerPaletteInstrument::ColorpickerPaletteInstrument(QObject *parent) :
    AbstractInstrument(parent){

}

ColorpickerPaletteInstrument::ColorpickerPaletteInstrument(AbstractInstrument *parent) :
    AbstractInstrument(parent){

}

void ColorpickerPaletteInstrument::mousePressEvent(QMouseEvent *event, ImageArea &imageArea){
    (void)imageArea;
    (void)event;
}

void ColorpickerPaletteInstrument::mouseMoveEvent(QMouseEvent *event, ImageArea &imageArea){
    (void)imageArea;
    (void)event;
}

void ColorpickerPaletteInstrument::mouseReleaseEvent(QMouseEvent *event, ImageArea &imageArea){
    (void)imageArea;
    (void)event;
}


void ColorpickerPaletteInstrument::showColorPalletteInstrument(ImageArea &imageArea){
    imageArea.setIsPaint(true);
    DataSingleton *dataSingleton = DataSingleton::Instance();
    QList<QColor> colorStack = dataSingleton->getColorStack();
    for(int i = 0; i < colorStack.size(); i++){
        QColorDialog::setCustomColor(colorStack.size() - i -1,colorStack[i]);
    }
    QColor color = QColorDialog::getColor(
                dataSingleton->getPrimaryColor(),
                &imageArea, "Pick a color",
                QColorDialog::DontUseNativeDialog);
    dataSingleton->pushColorStack(color);
    dataSingleton->setPrimaryColor(color);
    imageArea.emitPrimaryColorView();
    imageArea.setIsPaint(false);
    imageArea.emitRestorePreviousInstrument();
}


void ColorpickerPaletteInstrument::paint(ImageArea &imageArea, bool isSecondaryColor, bool){
    (void)imageArea;
    (void)isSecondaryColor;
}
