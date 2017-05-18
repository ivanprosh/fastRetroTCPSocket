#include "addresstable.h"
#include "global.h"
#include "ConnectionManager.h"
#include "plcsocketclient.h"

#include <QDebug>
#include <QRegExp>

namespace {
const int MaxColumns = 2;
}

AddressTable::AddressTable(QObject *parent)
    : QAbstractTableModel(parent)
{
    initialize();
}

void AddressTable::initialize()
{
    hash = new QHash<int,QByteArray>;
    hash->insert(ipRole, "ipAddr");
    hash->insert(statusRole, "status");

    //setHorizontalHeaderLabels(QStringList() << tr("IP адрес") << tr("Статус"));

    //добавим строки
    //items.push_back(QPair<QString,QString>("172.16.3.121:10000","Не активен"));

    int row(0);
    while(row < MAX_CONNECTIONS_COUNT){
        items.push_back(QPair<QString,QString>("",""));
        setData(index(row,ipColumn), QString(""), ipRole);
        setData(index(row,ipColumn), QString(""), statusRole);
        row++;
    }
}

void AddressTable::clear()
{
    //QStandardItemModel::clear();
    initialize();
}

//вызывается при смене адреса в таблице (через интерфейс QML)
void AddressTable::ipChange(const int curRow, const int curColumn, const QVariant &value)
{
    if(data(index(curRow,curColumn),ipRole) == value)
        return;
    if(ConnectionManager::instance()->isConnectActive(ConnectionManager::instance()->findClient(curRow)))
        return;

    QRegExp correctIp("^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\:(\\d+)");

    if(index(curRow,curColumn).isValid() && (correctIp.exactMatch(value.toString()))){
        if(correctIp.cap(1).toInt() < 255 || correctIp.cap(2).toInt() < 255 || correctIp.cap(3).toInt() < 255 || correctIp.cap(4).toInt() < 255){
            //setData(index(curRow,curColumn), value, Qt::UserRole+curColumn);
            setData(index(curRow,curColumn), value, ipRole);
            setData(index(curRow,curColumn), QString("Не активен"), statusRole);
        }
    }
    else if(value.toString().isEmpty()){
        //setData(index(curRow,curColumn), value, Qt::UserRole+curColumn);
        setData(index(curRow,curColumn), value, ipRole);
        setData(index(curRow,curColumn), QString(""), statusRole);
    }

}
/*
 * изменение табличных данных. создание соединений происходит здесь же
 */
bool AddressTable::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && (role == Qt::EditRole || role >= Qt::UserRole)) {
        qDebug() << role << " " << value.toString();

        Q_ASSERT(index.row()<=items.size());

            switch (role) {
            case ipRole:
            {
                if(items.at(index.row()).first == value.toString())
                    return false;
                //смотрим, была ли уже запись о клиенте
                PLCSocketClient* curClient = ConnectionManager::instance()->findClient(index.row());
                if(curClient){
                    //удаляем клиента
                    ConnectionManager::instance()->removeConnection(curClient);
                }

                items[index.row()].first = value.toString();

                if(items.at(index.row()).first.isEmpty()){
                    //если получили пустышку, значит было удаление сервера
                    return true;
                }

                //заполняем параметры сервера
                if(!ConnectionManager::instance()->canAddConnection()) {
                    qDebug() << "Достигнут максимум подключений";
                    return false;
                }
                QSharedPointer<PLCServer> plc = QSharedPointer<PLCServer>(new PLCServer);
                plc->setId(index.row());
                plc->setAddress(value.toString());
                //инициализируем клиента
                QSharedPointer<PLCSocketClient> client(new PLCSocketClient(ConnectionManager::instance()->clientId()));
                client->setServer(plc);
                ConnectionManager::instance()->addConnection(client);
                //сигнал извещает класс главного окна о необходимости привязки клиента
                emit SockClientAdd(client);
                break;
            }
            case statusRole:
                if(items.at(index.row()).second == value.toString())
                    return false;
                items[index.row()].second = value.toString();
                break;
            default:
                return false;
            }
        //}
        //обязательно вызываем, чтобы уведомить интерфейс об изменениях
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

int AddressTable::rowCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : items.size();
}


int AddressTable::columnCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : MaxColumns;
}

QVariant AddressTable::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) return QVariant();

    //qDebug() << index.row() << items.size();
    Q_ASSERT(index.row()<=items.size());
    switch (role) {
    case ipRole:
        return items.at(index.row()).first;
    case statusRole:
        return items.at(index.row()).second;
    default:
        Q_ASSERT(false);
    }
}

QHash<int, QByteArray> AddressTable::roleNames() const
{
    return *hash;
}
Qt::ItemFlags AddressTable::flags(const QModelIndex &index) const
{
    Qt::ItemFlags theFlags = QAbstractTableModel::flags(index);
    if (index.isValid())
        theFlags |= Qt::ItemIsSelectable|Qt::ItemIsEditable|
                    Qt::ItemIsEnabled;
    return theFlags;
}
QVariant AddressTable::headerData(int section,
        Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        switch (section) {
            case ipColumn: return tr("IP адрес");
            case statusColumn: return tr("Статус");
            default: Q_ASSERT(false);
        }
    }
    return section + 1;
}


