#pragma once
#include <QApplication>
#include "MainWindow.h"
#include <QTimer>


class UIWrapper : public QObject
{
	Q_OBJECT
public:
	UIWrapper(QObject* parent = nullptr) : QObject(parent)
	{
		w1 = nullptr;
		w2 = nullptr;

		w1 = new MainWindow("User1");
		// Create a single shot timer that creates both MainWindows in differend times
		// Create single shot with lambda
		QTimer::singleShot(3000, [this]() { 
			w2 = new MainWindow("User2"); 
			w2->show(); 
			connect(w2, &MainWindow::closeWindow, this, &UIWrapper::onWindowClosed); 
			});
		

		w1->show();
		

		connect(w1, &MainWindow::closeWindow, this, &UIWrapper::onWindowClosed);
		
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