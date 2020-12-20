/*
MIT License

Copyright (c) 2020 Maifee Ul Asad

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

#include "cropinstrument.h"
#include "../imagearea.h"
#include "../undocommand.h"
#include "math.h"

#include <QPainter>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QLabel>

CropInstrument::CropInstrument(QObject *parent) :
    AbstractSelection(parent){
    this->installEventFilter(this);
}

void CropInstrument::copyImage(ImageArea &imageArea){
    (void)imageArea;
}

void CropInstrument::cutImage(ImageArea &imageArea){
    (void)imageArea;
}

void CropInstrument::pasteImage(ImageArea &imageArea){
    (void)imageArea;
}

void CropInstrument::startAdjusting(ImageArea &imageArea){
    mImageCopy = *imageArea.getImage();
    mIsImageSelected = false;
}

void CropInstrument::startSelection(ImageArea &){

}

void CropInstrument::startResizing(ImageArea &imageArea){
    if (!mIsImageSelected)
    {
        clearSelectionBackground(imageArea);
    }
    if (mIsSelectionAdjusting)
    {
        mIsImageSelected = false;
    }
}

void CropInstrument::startMoving(ImageArea &imageArea){
    clearSelectionBackground(imageArea);
    if (mIsSelectionAdjusting)
    {
        mIsImageSelected = false;
    }
}

void CropInstrument::select(ImageArea &){

}

void CropInstrument::resize(ImageArea &){

}

void CropInstrument::move(ImageArea &){

}

void CropInstrument::completeSelection(ImageArea &imageArea){
    mSelectedImage = imageArea.getImage()->copy(mTopLeftPoint.x(),
                                                mTopLeftPoint.y(),
                                                mWidth, mHeight);
    showCropConfirmation(imageArea);
    emit sendEnableCopyCutActions(true);
}

void CropInstrument::completeResizing(ImageArea &imageArea){
    mSelectedImage = imageArea.getImage()->copy(mTopLeftPoint.x(),
                                                mTopLeftPoint.y(),
                                                mWidth, mHeight);
    showCropConfirmation(imageArea);
}

void CropInstrument::completeMoving(ImageArea &imageArea){
    if (mIsSelectionAdjusting)
    {
        mSelectedImage = imageArea.getImage()->copy(mTopLeftPoint.x(),
                                                   mTopLeftPoint.y(),
                                                   mWidth, mHeight);
    }
    showCropConfirmation(imageArea);
}

void CropInstrument::clearSelectionBackground(ImageArea &imageArea){
   (void) imageArea;
}

void CropInstrument::clear(){
    mSelectedImage = QImage();
    emit sendEnableCopyCutActions(false);
}

void CropInstrument::paint(ImageArea &imageArea, bool, bool){
    if (mIsSelectionExists && !mIsSelectionAdjusting)
    {
        imageArea.setEdited(true);
        imageArea.update();
    }
}

void CropInstrument::showMenu(ImageArea &){

}

void CropInstrument::showCropConfirmation(ImageArea &imageArea){
    int message = QMessageBox::question(
                qobject_cast<QWidget *> (parent()),
                tr("Crop confirmation"),
                tr("Crop this area ?"),

                QMessageBox::Yes |
                QMessageBox::No |
                QMessageBox::Cancel,

                QMessageBox::Cancel );
    if(message == QMessageBox::Yes){
        mSelectedImage = imageArea.getImage()->copy(mTopLeftPoint.x(), mTopLeftPoint.y(), mWidth, mHeight);
        imageArea.setImage(mSelectedImage);
        imageArea.update();
        return;
    }
    if(message == QMessageBox::No){
        return;
    }
    if(message == QMessageBox::Cancel){
        clear();
        return;
    }
}
