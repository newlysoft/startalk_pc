﻿//
// Created by cc on 18-12-10.
//

#include "SystemTray.h"
#include <QMenu>
#include <QAction>
#include <QWidget>
#include <QApplication>
#include <QFile>
#include <QtConcurrent>
#include <QScreen>
#include "../interface/view/IUILoginPlug.h"
#include "MainWindow.h"
#include "../Platform/AppSetting.h"
#include "../Platform/Platform.h"
#include "../quazip/JlCompress.h"
#include "MessageManager.h"
#include "../CustomUi/QtMessageBox.h"

extern bool _bSystemRun;
SystemTray::SystemTray(MainWindow* mainWnd)
    : QObject(mainWnd), _pSysTrayIcon(nullptr),
      _timer(nullptr), _pMainWindow(mainWnd),
      _timerCount(0)
{
    _pSysTrayIcon = new QSystemTrayIcon(this);

#if defined(_STARTALK)
    _pSysTrayIcon->setIcon(QIcon(":/QTalk/image1/StarTalk.png"));
#elif defined(_QCHAT)
    _pSysTrayIcon->setIcon(QIcon(":/QTalk/image1/Qchat.png"));
#else
    _pSysTrayIcon->setIcon(QIcon(":/QTalk/image1/QTalk.png"));
#endif

    _pSysTrayIcon->show();
    //
    _popWnd = new SystemTrayPopWnd(_pMainWindow);
    _popWnd->setWindowFlags(_popWnd->windowFlags() | Qt::Popup);
    _popWnd->setFixedWidth(280);
    auto* pSysTrayMenu = new QMenu;
    pSysTrayMenu->setAttribute(Qt::WA_TranslucentBackground, true);
    auto* sysQuitAct = new QAction(tr("系统退出"));
    auto* showWmdAct = new QAction(tr("显示面板"));
    auto* autoLoginAct = new QAction(tr("自动登录"));
    auto* sendLog = new QAction(tr("快速反馈日志"));
    if (_pMainWindow->getLoginPlug())
    {
        bool isAuto = _pMainWindow->getLoginPlug()->getAutoLoginFlag();
        if (isAuto) autoLoginAct->setText(tr("取消自动登录"));
    }
    pSysTrayMenu->addAction(autoLoginAct);
    pSysTrayMenu->addAction(sendLog);
    pSysTrayMenu->addAction(showWmdAct);
    pSysTrayMenu->addSeparator();
    pSysTrayMenu->addAction(sysQuitAct);
#ifdef _WINDOWS
    _pSysTrayIcon->setContextMenu(pSysTrayMenu);
#endif
    _timer = new QTimer;
    _timer->setInterval(500);

    //
    connect(_pSysTrayIcon, &QSystemTrayIcon::activated, this, &SystemTray::activeTray);
    connect(_popWnd, &SystemTrayPopWnd::sgQuit, _pMainWindow, &MainWindow::systemQuit);
    connect(_popWnd, &SystemTrayPopWnd::sgJumtoSession, _pMainWindow, &MainWindow::sgJumtoSession);
    connect(_popWnd, &SystemTrayPopWnd::sgJumtoSession, [this](const StSessionInfo&){_pMainWindow->wakeUpWindow();});
    connect(_popWnd, &SystemTrayPopWnd::sgFeedback, this, &SystemTray::onSendLog);
    connect(_popWnd, &SystemTrayPopWnd::sgCancelAlert, this, &SystemTray::stopTimer);
    connect(_popWnd, &SystemTrayPopWnd::sgStartTimer, [this](){
        if(!_timer->isActive())
        {
            _timerCount = 0;
            _timer->start();
        }
        if(AppSetting::instance().getStrongWarnFlag())
            QApplication::alert(_pMainWindow);
    });
    connect(this, &SystemTray::sgShowUnreadMessage, _popWnd, &SystemTrayPopWnd::onNewMessage);

    connect(_timer, &QTimer::timeout, this, &SystemTray::onTimer);

	connect(sysQuitAct, &QAction::triggered, _pMainWindow, &MainWindow::systemQuit);
	connect(sendLog, &QAction::triggered, this, &SystemTray::onSendLog);
	connect(showWmdAct, &QAction::triggered, [this]()
		{
			stopTimer();
			_pMainWindow->wakeUpWindow();
		});
	connect(autoLoginAct, &QAction::triggered, [this, autoLoginAct]()
		{
			stopTimer();
			if (_pMainWindow->getLoginPlug())
			{
				bool isAuto = _pMainWindow->getLoginPlug()->getAutoLoginFlag();
				_pMainWindow->setAutoLogin(!isAuto);
				if (isAuto)
					autoLoginAct->setText(tr("自动登录"));
				else
					autoLoginAct->setText(tr("取消自动登录"));
			}
		});

    //
    bool isSupportsMessages = _pSysTrayIcon->supportsMessages();
    AppSetting::instance().setNativeMessagePromptEnable(isSupportsMessages);
	//

	auto* hoverTimer = new QTimer;
	hoverTimer->setInterval(1000);
	connect(hoverTimer, &QTimer::timeout, [this, pSysTrayMenu]() {

        bool bSysTray = _pSysTrayIcon->geometry().contains(QCursor::pos());
	    bool bPopWnd =  _popWnd->geometry().contains(QCursor::pos());
		if ((!_popWnd->isVisible() && bSysTray) ||
                (_popWnd->isVisible() && (bSysTray || bPopWnd))) {


			static unsigned char flag = 0;
			if (flag++ % 2)
				return;

			if (!pSysTrayMenu->isVisible() &&_popWnd->hasNewMessage() && !_popWnd->isVisible())
			{
				QRect rect = _pSysTrayIcon->geometry();
                auto pos = QCursor::pos();
#ifdef _LINUX
                QScreen *screen = nullptr;
    auto lstScreens = QApplication::screens();
    for(QScreen* tmps : lstScreens)
    {
        if(tmps->geometry().contains(pos))
        {
            screen = tmps;
            break;
        }
    }

#else
                QScreen *screen = QApplication::screenAt(pos);
#endif
				auto screenRect = screen->availableGeometry();
				auto x = qMin(screenRect.right() - _popWnd->width() - 10, rect.x());

				_popWnd->show();
#ifdef _WINDOWS
                _popWnd->move(x, rect.y() - _popWnd->height() - 2);
#else
                _popWnd->move(x, rect.bottom());
#endif
			}
		}
		else {
			if (_popWnd->isVisible())
				_popWnd->setVisible(false);
		}
	});
	hoverTimer->start();

}

SystemTray::~SystemTray() = default;

/**
  * @函数名   系统托盘事件处理
  * @功能描述
  * @参数
  * @author   cc
  * @date     2018/11/09
  */
void SystemTray::activeTray(QSystemTrayIcon::ActivationReason reason)
{    //
    if (reason == QSystemTrayIcon::Trigger )
    {
        if (_pMainWindow)
			_pMainWindow->wakeUpWindow();
    }
}

//void SystemTray::onRecvMessage()
//{
//    //
//    if(!_pMainWindow->isActiveWindow())
//    {
//        if(!_timer->isActive() )
//        {
//            _timerCount = 0;
//            _timer->start();
//        }
//
//        if(AppSetting::instance().getStrongWarnFlag())
//        {
//            QApplication::alert(_pMainWindow);
//        }
//
//        _popWnd->setCancelAlertBtnVisible(true);
//    }
//}

void SystemTray::onShowNotify(const QTalk::StNotificationParam &param) {

    if(_pSysTrayIcon)
        _pSysTrayIcon->showMessage(param.title.data(), param.message.data(), QIcon(param.icon.data()));
}

void SystemTray::onTimer() {

    _timerCount ++;
	
    if(_timerCount % 2 == 0)
    {
#if defined(_STARTALK)
        _pSysTrayIcon->setIcon(QIcon(":/QTalk/image1/StarTalk.png"));
#else
        _pSysTrayIcon->setIcon(QIcon(":/QTalk/image1/QTalk.png"));
#endif
    } else {
#if defined(_STARTALK)
        _pSysTrayIcon->setIcon(QIcon(":/QTalk/image1/starTalkTip.png"));
#elif defined(_QCHAT)
        _pSysTrayIcon->setIcon(QIcon(":/QTalk/image1/Qchat.png"));
#else
        _pSysTrayIcon->setIcon(QIcon(":/QTalk/image1/redPoint.png"));
#endif
    }
}

void SystemTray::stopTimer() {
    _popWnd->setCancelAlertBtnVisible(false);

    if(_popWnd->isVisible())
    {
        _popWnd->setVisible(false);
        _pMainWindow->wakeUpWindow();
    }

    if(_timer->isActive())
    {
        _timer->stop();
#if defined(_STARTALK)
        _pSysTrayIcon->setIcon(QIcon(":/QTalk/image1/StarTalk.png"));
#elif defined(_QCHAT)
        _pSysTrayIcon->setIcon(QIcon(":/QTalk/image1/Qchat.png"));
#else
        _pSysTrayIcon->setIcon(QIcon(":/QTalk/image1/QTalk.png"));
#endif
    }
}

void SystemTray::onWndActived()
{
//    stopTimer();
}

/**
 *
 */
void SystemTray::onSendLog()
{
    _popWnd->setVisible(false);

    int btn = QtMessageBox::question(_pMainWindow, tr("提醒"), "是否反馈日志 ? ");

    if(btn == QtMessageBox::EM_BUTTON_NO)
        return;

    QtConcurrent::run([this]() {
#ifdef _MACOS
        pthread_setname_np("ChatViewMainPanel::packAndSendLog");
#endif
        //db 文件
        QString logBasePath;
        logBasePath = QString::fromStdString(Platform::instance().getAppdataRoamingPath()) + "/logs";
        // zip
        QString logZip = logBasePath + "/log.zip";
        if (QFile::exists(logZip))
            QFile::remove(logZip);
        //
        bool ret = JlCompress::compressDir(logZip, logBasePath);
        if (ret) {
            if (_pMainWindow && _pMainWindow->_pMessageManager) {
                _pMainWindow->_pMessageManager->sendLogReport("quick report", logZip.toStdString());
            }
        }
    });
}

void SystemTray::onMessageClicked() {
    if(_pMainWindow && !_pMainWindow->isVisible())
        _pMainWindow->wakeUpWindow();
}

//
void SystemTray::onAppDeactivated() {

}