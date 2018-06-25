/*! \brief Convert a series of images into 4 pictures "time-slice"
 * \file tlstripmainwindow.h
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

#ifndef TLSTRIPMAINWINDOW_H
#define TLSTRIPMAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>
#include <QPixmap>
#include <QStringList>



namespace Ui {
class TLStripMainWindow;
}

class TLStripThread : public QThread {
	Q_OBJECT
public:
	TLStripThread();
	~TLStripThread();
	QImage getProgressImage();
	int getProgress();
	void run() Q_DECL_OVERRIDE;
	void setFileList(QStringList list);

private:
	QWaitCondition mWaitCondition;
	QMutex mMutex;
	QStringList fileList;
	int mProgress;
	bool mIsRunning;
	bool mRun;

	QPixmap pixH ;
	QPixmap pixH2;
	QPixmap pixV ;
	QPixmap pixV2;
};


class TLStripMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit TLStripMainWindow(QWidget *parent = 0);
	~TLStripMainWindow();

private slots:
	void on_openButton_clicked();
	void slot_timeout();
private:
	QTimer mTimer;
	Ui::TLStripMainWindow *ui;
	TLStripThread thread;

};



#endif // TLSTRIPMAINWINDOW_H

