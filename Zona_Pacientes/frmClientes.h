#ifndef FRMCLIENTES_H
#define FRMCLIENTES_H


#include <QDialog>
#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "Busquedas/frmbuscarpoblacion.h"
#include "configuracion.h"
#include "frmfichapaciente.h"
#include "cliente.h"
#include "sqlcalls.h"



namespace Ui {
class frmClientes;
}

class frmClientes : public QDialog
{
    Q_OBJECT

public:
    explicit frmClientes(Configuracion *oConfiguracion, QWidget *parent = 0);
    ~frmClientes();
    bool Altas;

public slots:
    void LLenarCampos();
    void LLenarCliente();
    void VaciarCampos();
   // void SetId_Cliente(int id_Cliente);

signals:
    void enviahistoriaynombre(int,QString);


private slots:
    void on_btnSiguiente_clicked();

    void on_btnAnterior_clicked();

    void on_btnGuardar_clicked();

    void on_btnAnadir_clicked();

    void txtPrimerApellido_editingFinished();

    void txtSegundoApellido_editingFinished();

    void txtcNombre_editingFinished();

    void txtcPoblacion_editingFinished();

    void txtcProvincia_editingFinished();

    void txtcCifNif_editingFinished();

    void on_btnEditar_clicked();
    void bloquearCampos();
    void desbloquearCampos();

    void on_btnDeshacer_clicked();

    void on_btnBorrar_clicked();

    void on_btnBuscar_clicked();

    void txtcCp_editingFinished();

    void txtcCPFactura_editingFinished();

    void txtcPoblacionFactura_editingFinished();

    void txtcCpPoblacionAlmacen_editingFinished();

    void txtcPoblacionAlmacen_editingFinished();

    void TablaDeudas_clicked(const QModelIndex &index);

    void txtrRiesgoPermitido_editingFinished();

    void on_btnFichaPaciente_clicked();


private:
    Ui::frmClientes *ui;
    QSqlQueryModel *modelFP;
    QSqlQueryModel *modelFacturas;
    QSqlQueryModel *modelPoblaciones;
    QSqlDatabase dbCliente;
    FrmBuscarPoblacion BuscarPoblacion;
    QSqlQuery tbpaciente;
    Cliente* oCliente;
    SqlCalls *llamadasSQL;


};
#endif