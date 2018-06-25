/*! \brief Convert a series of images into 4 pictures "time-slice"
 * \file tlstripmainwindow.cpp
 * \copyright Christophe Seyve \em cseyve@free.fr
 */

/*
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "tlstripmainwindow.h"
#include "ui_tlstripmainwindow.h"

#include <QMainWindow>

#include <QFileDialog>
#include <QFileInfo>
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <stdio.h>

TLStripMainWindow::TLStripMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::TLStripMainWindow)
{
	ui->setupUi(this);

	connect(&mTimer, SIGNAL(timeout()), this, SLOT(slot_timeout()));
}

TLStripMainWindow::~TLStripMainWindow()
{
	delete ui;
}

void TLStripMainWindow::slot_timeout() {
	int progress = thread.getProgress();
	ui->progressBar->setValue(progress);
	if(progress >= 100) {
		mTimer.stop();
	}
	// update image
	QPixmap pixH = QPixmap::fromImage(thread.getProgressImage());
	ui->imageLabel->setPixmap(pixH.scaled(ui->imageLabel->size()));

}

void TLStripMainWindow::on_openButton_clicked()
{
	QStringList fileList = QFileDialog::getOpenFileNames(this, tr("Image files"), ".", "Images (*.JPG *.jpg)");
	if(fileList.isEmpty()) {return; }

	if(!thread.isRunning())
	{
		fprintf(stdout, "Start thread\n");
		thread.start();
	}

	thread.setFileList(fileList);

	mTimer.start(5000);
}



TLStripThread::TLStripThread()
	: QThread() {
	mRun = mIsRunning = false;
	mProgress = 100;

}
TLStripThread::~TLStripThread() {
	mRun = false;
	while(mIsRunning) {
		usleep(1000);
	}
}

QImage TLStripThread::getProgressImage() {
	QPixmap scaled;
	if( !pixH.isNull() ) {
		scaled = pixH.scaled(600,400);
	}
	return scaled.toImage();
}


void TLStripThread::setFileList(QStringList list) {
	fileList = list;
	// unlock thread
	//mMutex.lock();
	fprintf(stdout, "%d items in list => wake thread\n", fileList.count());
	mWaitCondition.wakeAll();

}

void TLStripThread::run()
{
	fprintf(stdout, "Thread started\n");
	mIsRunning = true;
	mRun = true;
	while (mRun) {
		// Wait on condition
		mMutex.lock();
		//fprintf(stdout, "    waiting...\n");fflush(stdout);
		mWaitCondition.wait(&mMutex, 100);
		mMutex.unlock();
		//fprintf(stdout, "     I'm awake!!\n");fflush(stdout);
		mProgress = 100;
		if(fileList.isEmpty()) {
			//fprintf(stdout, "Empty list\n");fflush(stdout);
			continue;
		}


		QFileInfo fi(fileList.at(0));
		QString dirName = fi.absoluteDir().absolutePath();
		QImage out;
		out.load(fileList.at(0));
		fprintf(stdout, "%d items in list\n", fileList.count());fflush(stdout);


		pixH = QPixmap::fromImage(out);
		pixH2 = QPixmap::fromImage(out);
		pixV = QPixmap::fromImage(out);
		pixV2 = QPixmap::fromImage(out);

		QPainter pH(&pixH);
		QPainter pH2(&pixH2);
		QPainter pV(&pixV);
		QPainter pV2(&pixV2);
		int imwidth = out.width();
		int imheight = out.height();
		int nb = fileList.count();
		int dx = imwidth / nb + 1;
		int dy = imheight / nb + 1;
		// open all the files
		QStringList::iterator it;
		int idx = 0;
		for(it = fileList.begin(); it != fileList.end(); it++, idx++) {
			QString filename = (*it);
			mProgress = idx * 100 / nb;
			fprintf(stdout, "\rProcessing %4d/%4d '%s'...", idx, nb, qPrintable(filename));fflush(stdout);
			QImage current;
			if (current.load(filename)) {
				int x = idx * imwidth / nb;
				int y = idx * imheight / nb;
				int x2 = (nb - 1 - idx)* imwidth / nb;
				int y2 = (nb - 1 - idx)* imheight / nb;

				QImage cutH = current.copy(QRect(x, 0, dx, imheight));
				pH.drawImage(x, 0, cutH);
				QImage cutH2 = current.copy(QRect(x2, 0, dx, imheight));
				pH2.drawImage(x2, 0, cutH2);
				QImage cutV = current.copy(QRect(0, y, imwidth, dy));
				pV.drawImage(0, y, cutV);
				QImage cutV2 = current.copy(QRect(0, y2, imwidth, dy));
				pV2.drawImage(0, y2, cutV2);
			}
		}
		pH.end();
		pV.end();
		pH2.end();
		pV2.end();
		mProgress = 100;
		pixH.save( dirName + "/TimelapseStripH.jpg",  "JPG", 95);
		pixV.save( dirName + "/TimelapseStripV.jpg",  "JPG", 95);
		pixH2.save(dirName + "/TimelapseStripH2.jpg", "JPG", 95);
		pixV2.save(dirName + "/TimelapseStripV2.jpg", "JPG", 95);
		fileList.clear();
	}
	mIsRunning = false;
}

int TLStripThread::getProgress() {
	return mProgress;
}














