#ifndef FRMFACTURAS_H
#define FRMFACTURAS_H

#include "Auxiliares/Globlal_Include.h"
#include "configuracion.h"
#include "factura.h"
#include "Zona_Pacientes/cliente.h"
#include "Auxiliares/table_helper.h"
namespace Ui {
class frmFacturas;
}

class frmFacturas : public QDialog
{
    Q_OBJECT
    
public slots:
    void LLenarCampos();
    void LLenarCamposCliente();
    void LLenarFactura();
    void VaciarCampos();
    void BloquearCampos();
    void DesbloquearCampos();

public:
    explicit frmFacturas(Configuracion *o_config, QWidget *parent = 0);
    ~frmFacturas();
    Factura *oFactura;
    Cliente *oCliente1;
    
private slots:
    void lineasVentas();

    void on_btnSiguiente_clicked();

    void on_btnGuardar_clicked();

    void on_btnAnadir_clicked();

    void calcularTotalLinea();

    void on_btnDeshacer_clicked();

    void on_btnAnterior_clicked();

    void on_botBuscarCliente_clicked();

    void on_btnBuscarArt_clicked();

    void on_tablaBuscaArt_doubleClicked(const QModelIndex &index);

    void on_chklRecargoEquivalencia_stateChanged(int arg1);

    void on_btnBuscar_clicked();

    void on_txtcCodigoCliente_lostFocus();

    void on_btnImprimir_clicked();

    void RellenarDespuesCalculo();

    void on_botBorrador_clicked();

    void totalChanged(QString total);    
private:
    Ui::frmFacturas *ui;
    QSqlDatabase dbEmp;
    QSqlDatabase dbTerra;
    QSqlQueryModel *modelFP;
    QSqlQueryModel *modArt;
    bool Altas;
    QSqlQueryModel *ModelLin_fac;
    QHeaderView *Cabecera;
    double importe;
    Configuracion *o_configuracion;
    Table_Helper helper;


public:
    QString TextoNumero(QString cTexto);

};

#endif // FRMFACTURAS_H
