#include "MkLsHelper.hpp"
#include "utilsClient.hpp"

#include "QsLog.h"
#include "QsLogDest.h"
#include <QDir>
#include <iostream>

MkLsHelper::MkLsHelper(const QString& host, const int& port)
  : mhost(host), mport(port)
{
  setSocketOption(QAbstractSocket::KeepAliveOption, 1);
  setRequestPatterns();
  SETUP_LOGGING(); //FIXME: logging
}

MkLsHelper::~MkLsHelper()
{
  QAbstractSocket::disconnect();
}

void MkLsHelper::setRequestPatterns()
{
  mrequestMap[Host] = "GET hosts\n"
      "Columns: name state last_state_change check_command plugin_output\n"
      "Filter: name = %1\n\n";
  mrequestMap[Service] = "GET services\n"
      "Columns: host_name service_description state last_state_change check_command plugin_output\n"
      "Filter: host_name = %1\n\n";
}

bool MkLsHelper::connectToService()
{
  disconnectFromHost();
  qDebug() << tr("Connecting to %1:%2...").arg(mhost).arg(mport);
  QAbstractSocket::connectToHost(mhost, mport, QAbstractSocket::ReadWrite);
  if (!QAbstractSocket::waitForConnected(DefaultTimeout)) {
      handleFailure();
      return false;
    }
  return true;
}


void MkLsHelper::disconnectFromService()
{
  QAbstractSocket::disconnectFromHost();
}

bool MkLsHelper::requestData(const QString& host, const ReqTypeT& reqType)
{
  qint32 nb;
  if (!isConnected()) {
      connectToService();
    }
  if (!isConnected() ||
      (nb = QAbstractSocket::write(mrequestMap[reqType].arg(host).toAscii())) <= 0 ||
      !QAbstractSocket::waitForBytesWritten(DefaultTimeout)) {
      handleFailure();
      return false;
    }
  qDebug() << tr("%1 bytes written").arg(nb);
  return true;
}

bool MkLsHelper::recvData(const ReqTypeT& reqType)
{
  if (!QAbstractSocket::waitForReadyRead(DefaultTimeout)) {
      handleFailure();
      return false;
    }
  QString chkid = "";
  MonitorBroker::CheckT check;
  QString entry;
  QTextStream buffer(this);
  while (!((entry = buffer.readLine()).isNull())) {
      if (entry.isEmpty()) continue;
      QStringList fields = entry.split(";");
      chkid.clear();
      if (reqType == Host) {
          if (fields.size() != 5) {
              return false;
            }
          chkid = fields[0].toLower();
          check.id = check.host = fields[0].toStdString();
          check.status = fields[1].toInt();
          check.last_state_change = fields[2].toStdString();
          check.check_command = fields[3].toStdString();
          check.alarm_msg = fields[4].toStdString();
        } else if (reqType == Service) {
          if (fields.size() != 6)
            return false;
          QString hostname = fields[0];
          chkid = ID_PATTERN.arg(hostname, fields[1]).toLower();
          check.host = hostname.toStdString();
          check.id = chkid.toStdString();
          check.status = fields[2].toInt();
          check.last_state_change = fields[3].toStdString();
          check.check_command = fields[4].toStdString();
          check.alarm_msg = fields[5].toStdString();
        } else {
          QAbstractSocket::setErrorString(tr("Bad request type: %1").arg(reqType));
          return false;
        }
      mldchecks.insert(chkid, check);
    }
  return true;
}

bool MkLsHelper::loadHostData(const QString& host)
{
  mldchecks.clear();
  bool succeed;
  succeed = connectToService() &&
      requestData(host, Host) &&
      recvData(Host);
  disconnectFromService();

  succeed = succeed && connectToService() &&
      requestData(host, Service)&&
      recvData(Service);
  disconnectFromService();
  return true;
}

bool MkLsHelper::findCheck(const QString& id, CheckListCstIterT& check)
{
  check = mldchecks.find(id.toLower());
  if (check != mldchecks.end()) {
      return true;
    }
  return false;
}

void MkLsHelper::handleFailure(QAbstractSocket::SocketError error)
{
  switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
      QAbstractSocket::setErrorString(tr("The connection has been closed by the remote host.\n"));
      break;
    case QAbstractSocket::HostNotFoundError:
      QAbstractSocket::setErrorString(tr("The host not found: %1. Please check the "
                                         "host name and port settings.\n").arg(mhost));
      break;
    case QAbstractSocket::ConnectionRefusedError:
      QAbstractSocket::setErrorString(tr("The connection was refused by the server. "
                                         "Make sure that Mk Livestatus is running on %1:%2.\n").arg(mhost).arg(mport));
      break;
    default:
      QAbstractSocket::setErrorString(tr("The following error occurred: %1.\n")
                                      .arg(QAbstractSocket::errorString()));
    }
}

