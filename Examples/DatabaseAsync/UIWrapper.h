#pragma once
#include <QApplication>
#include "MainWindow.h"


class UIWrapper : public QObject
{
	Q_OBJECT
public:
	UIWrapper(QObject* parent = nullptr) : QObject(parent)
	{
		w1 = new MainWindow("User1");
		w2 = new MainWindow("User2");

		w1->show();
		w2->show();

		connect(w1, &MainWindow::closeWindow, this, &UIWrapper::onWindowClosed);
		connect(w2, &MainWindow::closeWindow, this, &UIWrapper::onWindowClosed);
	}
	~UIWrapper() {}
private slots:
	void onWindowClosed()
	{
		MainWindow* w = qobject_cast<MainWindow*>(sender());
		delete w;
		if (w == w1)
			w1 = nullptr;
		else
			w2 = nullptr;
	}
private:
	MainWindow* w1;
	MainWindow* w2;
};