﻿#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
#ifndef _CHATMAINWGT_H_
#define _CHATMAINWGT_H_

#include <QListWidget>
#include <QStandardItemModel>
#include <QTime>
#include <QMultiMap>
#include <QMenu>
#include <QMutexLocker>
#include <set>
#include <vector>
#include "../include/CommonDefine.h"
#include "NativeChatStruct.h"
#include "NewMessageTip.h"
#include "AtMessageTip.h"
#include "../entity/im_message.h"
#include "../include/STLazyQueue.h"
#include "ChatMainSoreModel.h"

class ChatViewItem;
class MessageItemBase;
class FileSendReceiveMessItem;
class VideoMessageItem;
class ChatMainDelegate;
class ChatMainWgt : public QListView
{
	Q_OBJECT
public:
	explicit ChatMainWgt(ChatViewItem *pViewItem);
	~ChatMainWgt() override;

public:
    void recvFileProcess(const double &speed, const double &dtotal, const double &dnow, const double &utotal, const double &unow, const std::string &key);
	void setHasNotHistoryMsgFlag(bool hasHistory);
    void recvBlackListMessage(const QString& messageId);
    void clearData();
    // 处理时间信息判断是否显示 超一分钟显示
    void showMessageTime(const QString& messageId, const QInt64& strTime);
    void onShowMessage(StNetMessageResult info, int);
    //
    void scrollToItem(QStandardItem* pItem);

private:
    void connects();

Q_SIGNALS:
	void showTipMessageSignal(const QString&, int, const QString&, QInt64);
	void adjustItems();
	void sgSelectItem();
	void sgSelectedSize(unsigned int);
	void sgUploadShareMsgSuccess(const QString&, int type, const QString&);
	void sgImageDownloaded(const QString&, const QString&);
	void sgJumTo();

public:
    void setShareMessageState(bool flag);
    void onShareMessage();
    void setConnectState(bool isConnected) {_connectState = isConnected; };
    void onUserMedalChanged(const std::set<std::string>& changedUser);

protected:
	void resizeEvent(QResizeEvent *e) override;
	void wheelEvent(QWheelEvent *e) override;
	bool event(QEvent* e) override ;
    void showEvent(QShowEvent* e) override;

private slots:
    void onRecvFRileProcess(double speed, double dtotal, double dnow, double utotal, double unow, std::string key);
    void onRecvReadState(const std::map<std::string, QInt32 >& readStates);
    void onMState(const QString& msgId, const long long& time);
    void updateRevokeMessage(const QString& fromId, const QString& messageId, const long long&);
    void onForwardAct(bool);
	void onScrollBarChanged(int val);
	void onImageDownloaded(const QString& msgId, const QString& path);
	void onCustomContextMenuRequested(const QPoint &pos);

private:
    void showTipMessage(const QString& messageId, int type, const QString& content, QInt64 t);
    void saveAsImage(const QString &oldFilePath);

private:
    void onCopyAction(bool);
    void onSaveAsAction(bool);
    void onRevokeAction(bool);
    void onQuoteAct(bool);
    void onCollectionAct(bool);
    void onQRCodeAct(bool);
    void onAdjustItems();
    void selectionChanged(const QItemSelection &, const QItemSelection &) override;
    void onItemChanged();
    void onDisconnected();
    void onSendMessageFailed(const QString& msgId);

public slots:
    void onItemCheckChanged(bool);

Q_SIGNALS:
    void sgRecvFRileProcess(double speed, double dtotal, double dnow, double utotal, double unow, std::string key);
    void gotReadStatueSignal(const std::map<std::string, QInt32 >& readStates);
    void sgDisConnected();
    void updateRevokeSignal(const QString& fromId, const QString& messageId, const long long&);
    void sgSendFailed(const QString& msgId);
    void sgGotMState(const QString& msgId, const long long& time);

private:
    void downloadImage(const QString& msgId, const QString& link, int width, int height);
    void downloadEmoticon(const QString& msgId, const QString& pkgid, const QString& shortCut);

public:
	ChatViewItem* _pViewItem;

    bool _connectState = false;

private:
    QStandardItemModel* _pSrcModel;
    ChatMainSoreModel*  _pModel;

private:

//    QTime         _downLoadElapsed; //用于进度条
    //long long      downloadProcess = 0;
    bool          _hasnotHistoryMsg;
    QMenu         *_pMenu;

private:
	NewMessageTip* _pNewMessageTipItem;
	AtMessageTip*   _pAtMessageTipItem;

private:
	int _oldScrollBarVal;
    long long downloadProcess = 0;
    STLazyQueue<bool> *_resizeQueue{};
    STLazyQueue<bool> *_selectItemQueue{};

private:
    bool _selectEnable{};

private:
    QAction* saveAsAct;
    QAction* copyAct;
    QAction* quoteAct;
    QAction* forwardAct;
    QAction* revokeAct;
    QAction* collectionAct;
    QAction* shareMessageAct;
    QAction* qrcodeAct;

private:
	QMap<QString, MessageItemBase*> _mapItemWgt;
	QMap<QString, QStandardItem*>   _mapItem;
	QMap<QString, QStandardItem*>   _mapTimeItem;
	std::list<QInt64>               _times;

    ChatMainDelegate*               _pDelegate{};

public:
	enum {EM_JUM_INVALID, EM_JUM_BOTTOM, EM_JUM_ITEM, EM_JUM_TOP};

private:
    int _jumType = EM_JUM_INVALID;
	QModelIndex _jumIndex;

public:
	void jumTo();
};

#endif//_CHATMAINWGT_H_
