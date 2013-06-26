/*
 * Base.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 24-05-2012                                                 #
#                                                                          #
# This file is part of RealOpInsight (http://RealOpInsight.com) authored   #
# by Rodrigue Chakode <rodrigue.chakode@gmail.com>                         #
#                                                                          #
# RealOpInsight is free software: you can redistribute it and/or modify    #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# The Software is distributed in the hope that it will be useful,          #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with RealOpInsight.  If not, see <http://www.gnu.org/licenses/>.   #
#--------------------------------------------------------------------------#
 */


#include "Base.hpp"
#include "Auth.hpp"
#include "Settings.hpp"
#include "Preferences.hpp"
#include "core/MonitorBroker.hpp"
#include "core/ns.hpp"
#include "utilsClient.hpp"
#include "JsHelper.hpp"
#include <QtScript/QScriptEngine>

const QString Settings::UPDATE_INTERVAL_KEY = "/Monitor/updateInterval";
const QString Settings::ADM_UNSERNAME_KEY = "/Auth/admUser";
const QString Settings::OP_UNSERNAME_KEY = "/Auth/opUsername";
const QString Settings::ADM_PASSWD_KEY = "/Auth/admPasswd";
const QString Settings::OP_PASSWD_KEY = "/Auth/opPasswd";
const QString Settings::SRC_BUCKET_KEY = "/Sources/buckets";

Settings::Settings(): QSettings(PROJECT.toLower(), APP_NAME.toLower().replace(" ", "-"))
{
  Q_INIT_RESOURCE(ngrt4n);
  SourceT src; loadSource(0, src);
  QString updateInterval = QSettings::value(Settings::UPDATE_INTERVAL_KEY).toString();
  QString admUser = QSettings::value(Settings::ADM_UNSERNAME_KEY).toString();
  QString admPasswd = QSettings::value(Settings::ADM_PASSWD_KEY).toString();
  QString opUser = QSettings::value(Settings::OP_UNSERNAME_KEY).toString();
  QString opPasswd = QSettings::value(Settings::OP_PASSWD_KEY).toString();

  if (updateInterval.isEmpty()) {
    QSettings::setValue(Settings::UPDATE_INTERVAL_KEY, QString::number(MonitorBroker::DefaultUpdateInterval));
  }
  if (admUser.isEmpty()) {
    QSettings::setValue(Settings::ADM_UNSERNAME_KEY, Auth::AdmUser);
  }
  if (admPasswd.isEmpty()) {
    QString passwd = QCryptographicHash::hash(Auth::AdmUser.toAscii(), QCryptographicHash::Md5) ;
    QSettings::setValue(Settings::ADM_PASSWD_KEY, passwd);
  }
  if (opUser.isEmpty()) {
    QSettings::setValue(Settings::OP_UNSERNAME_KEY, Auth::OpUser);
  }
  if (opPasswd.isEmpty()) {
    QString passwd = QCryptographicHash::hash(Auth::OpUser.toAscii(), QCryptographicHash::Md5) ;
    QSettings::setValue(Settings::OP_PASSWD_KEY, passwd);
  }
  translator = new QTranslator();
  translator->load("ngrt4n_la");
  qApp->installTranslator(translator);
  sync();
}

void Settings::setKeyValue(const QString & _key, const QString & _value)
{
  setValue(_key, _value) ;
  sync() ;
}


qint32 Settings::getUpdateInterval() const
{
  qint32 interval = QSettings::value(UPDATE_INTERVAL_KEY).toInt();
  return (interval > 0)? interval : MonitorBroker::DefaultUpdateInterval;
}

void Settings::setEntry(const QString& key, const QString& value)
{
  QSettings::setValue(key, value);
}

bool Settings::loadSource(const qint32& _idx, SourceT& _src)
{
  return setSource(QSettings::value(utils::sourceKey(_idx)).toString(), _src);
}

bool Settings::loadSource(const QString& _id, SourceT& _src)
{
  return setSource(QSettings::value(utils::sourceKey(_id)).toString(), _src);
}


bool Settings::setSource(const QString& _info, SourceT& _src)
{
  if (_info.isEmpty()) {
    return false;
  }

  JsonHelper jsHelper(_info);
  _src.id = jsHelper.getProperty("sid").toString();
  _src.mon_type = jsHelper.getProperty("mon_type").toInt32();
  _src.mon_url = jsHelper.getProperty("mon_url").toString();
  _src.auth = jsHelper.getProperty("auth").toString();
  _src.use_ls = jsHelper.getProperty("use_ls").toInt32();
  _src.ls_addr = jsHelper.getProperty("ls_addr").toString();
  _src.ls_port = jsHelper.getProperty("ls_port").toInt32();
  _src.verify_ssl_peer = jsHelper.getProperty("verify_ssl_peer").toInt32();
  return true;
}
