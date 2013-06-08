#ifndef MKLSHELPER_HPP
#define MKLSHELPER_HPP

#include "Base.hpp"
#include <QTcpSocket>

class LsHelper : public QTcpSocket
{
  Q_OBJECT
public:
  enum ReqTypeT{
    Host = 0,
    Service = 1
  };
  LsHelper(const QString& host, const int& port);
  ~LsHelper();

  bool connectToService(void);
  void disconnectFromService(void);
  bool requestData(const QString& host, const ReqTypeT& reqType);
  bool recvData(const ReqTypeT& reqType);
  bool loadHostData(const QString& host);
  bool findCheck(const QString& id, CheckListCstIterT& check);
  void clearData(void) {m_ldchecks.clear();}
  void setHost(const QString& host) {m_host = host;}
  void setPort(const int& port) {m_port = port;}
  bool isConnected() const {return state() == QAbstractSocket::ConnectedState;}

private:
  const static int DefaultTimeout = 50000; /* 5 seconds */
  QString m_host;
  qint32 m_port;
  RequestListT mrequestMap;
  CheckListT m_ldchecks;
  QString m_errorMsg;
  void setRequestPatterns();
  void handleNetworkFailure() {handleNetworkFailure(QAbstractSocket::error());}
  void handleNetworkFailure(QAbstractSocket::SocketError error);
};

#endif // MKLSHELPER_HPP
