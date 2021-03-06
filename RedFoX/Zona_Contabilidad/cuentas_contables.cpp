#include "cuentas_contables.h"

Cuentas_contables::Cuentas_contables(QObject *parent) :
    QObject(parent)
{
    id =0;
    activo = true;
    saldo =0;
    saldo_debe= 0;
    saldo_haber=0;
}

bool Cuentas_contables::anadir_cuenta()
{
//    QSqlQuery query_cuentas(Configuracion_global->contaDB);
//    query_cuentas.prepare("INSERT INTO plan_general "
//                          "(codigo_cta,descripcion,activo,saldo) "
//                          "VALUES (:codigo_cta,:descripcion,:activo,:saldo);");
//    query_cuentas.bindValue(":codigo_cta",this->codigo_cta);
//    query_cuentas.bindValue(":descripcion",this->descripcion);
//    query_cuentas.bindValue(":activo",this->activo);
//    query_cuentas.bindValue(":saldo",this->saldo);
    QString _error;
    QHash <QString,QVariant> h;
    h["codigo_cta"] = this->codigo_cta;
    h["descripcion"] = this->descripcion;
    h["activo"] = this->activo;
    h["saldo"] = this->saldo;
    h["cif_nif"] = this->cif_nif;
    h["saldo_debe"]=this->saldo_debe;
    h["saldo_haber"] = this->saldo_haber;
    h["direccion"] = this->direccion;
    h["poblacion"] = this->poblacion;
    h["provincia"] = this->provincia;
    h["pais"] = this->pais;

    int id = SqlCalls::SqlInsert(h,"plan_general",Configuracion_global->contaDB,_error);
    if(!id >0 )
    {
        QMessageBox::warning(qApp->activeWindow(),tr("Añadir cuentas contables"),
                             tr("Falló la inserción de la cuenta contable: %1").arg(_error));
        return false;
    } else
    {
        this->id = id;
        return true;
    }
}

void Cuentas_contables::guardar_cuenta()
{
    QSqlQuery query_cuentas(Configuracion_global->contaDB);
    query_cuentas.prepare("UPDATE plan_general SET "
                          "codigo_cta = :codigo_cta,"
                          "descripcion =:descripcion,"
                          "activo = :activo,"
                          "saldo = :saldo "
                          " WHERE id = :id;");
    query_cuentas.bindValue(":codigo_cta",this->codigo_cta);
    query_cuentas.bindValue(":descripcion",this->descripcion);
    query_cuentas.bindValue(":activo",this->activo);
    query_cuentas.bindValue(":saldo",this->saldo);
    query_cuentas.bindValue(":id",this->id);
    if(!query_cuentas.exec())
        QMessageBox::warning(qApp->activeWindow(),tr("Editar cuentas contables"),
                             tr("Falló la edición de la cuenta contable: %1").arg(query_cuentas.lastError().text()));
}

void Cuentas_contables::recuperar_cuenta(QString cuenta)
{
    QSqlQuery query_cuenta(Configuracion_global->contaDB);
    query_cuenta.prepare("select * from plan_general where codigo_cta = :cuenta ");
    query_cuenta.bindValue(":cuenta",cuenta);
    if(query_cuenta.exec()){
        query_cuenta.next();
        this->id = query_cuenta.record().value("id").toInt();
        this->activo = query_cuenta.record().value("activo").toBool();
        this->descripcion = query_cuenta.record().value("descripcion").toString();
        this->saldo = query_cuenta.record().value("saldo").toDouble();
    } else
        QMessageBox::warning(qApp->activeWindow(),tr("Gestión de cuentas de PGC"),
                             tr("Ocurrió un error al recuperar la cuenta: %1").arg(query_cuenta.lastError().text()));
}

void Cuentas_contables::recuperar_cuenta(int id)
{
    QSqlQuery query_cuenta(Configuracion_global->contaDB);
    query_cuenta.prepare("select * from plan_general where id = :id ");
    query_cuenta.bindValue(":id",id);
    if(query_cuenta.exec()){
        query_cuenta.next();
        this->id = query_cuenta.record().value("id").toInt();
        this->activo = query_cuenta.record().value("activo").toBool();
        this->afecta_a = query_cuenta.record().value("afecta_a").toString();
        this->codigo_balance = query_cuenta.record().value("codigo_balance").toString();
        this->descripcion = query_cuenta.record().value("descripcion").toString();
        this->desglose_balance = query_cuenta.record().value("desglose_balance").toString();
        this->saldo = query_cuenta.record().value("saldo").toDouble();
    } else
        QMessageBox::warning(qApp->activeWindow(),tr("Gestión de cuentas de PGC"),
                             tr("Ocurrió un error al recuperar la cuenta: %1").arg(query_cuenta.lastError().text()));
}

QString Cuentas_contables::completar_cuenta(QString cuenta_)
{
    int pos = cuenta_.indexOf(".");
    if(pos >0)
    {
        QString base = cuenta_.left(pos-1);
        QString resto = cuenta_.right(pos+1);

        int tamano_cuenta = Configuracion_global->digitos_cuentas_contables;
        int tamano = cuenta_.size()-1;
        for(int zero = 0;zero <tamano_cuenta - tamano;zero++)
        {
            resto.prepend("0");
        }
        return base.append(resto);
    }
    return cuenta_;
}

QString Cuentas_contables::nueva_cuenta()
{
    // TODO - Crear nueva cuenta contable
    return "XXXX";
}

void Cuentas_contables::clear()
{
    this->id = 0;
    this->activo = true;
    this->afecta_a ="";
    this->codigo_balance = "";
    this->codigo_cta = "";
    this->descripcion = "";
    this->desglose_balance = "";
    this->saldo = 0;


}

