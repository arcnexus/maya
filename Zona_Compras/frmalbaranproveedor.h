#ifndef FRMALBARANPROVEEDOR_H
#define FRMALBARANPROVEEDOR_H

#include <QDialog>
#include "../Auxiliares/Globlal_Include.h"
#include "../Auxiliares/table_helper.h"
#include "proveedor.h"

namespace Ui {
class FrmAlbaranProveedor;
}

class FrmAlbaranProveedor : public QDialog
{
    Q_OBJECT
    
public:
    explicit FrmAlbaranProveedor(QWidget *parent = 0, bool showCerrar = false);
    ~FrmAlbaranProveedor();
    void llenarProveedor(int id);
private slots:
    void totalChanged(double base , double dto ,double subTotal , double iva, double re, double total, QString moneda);
    void desglose1Changed(double base, double iva, double re, double total);
    void desglose2Changed(double base, double iva, double re, double total);
    void desglose3Changed(double base, double iva, double re, double total);
    void desglose4Changed(double base, double iva, double re, double total);
    void on_btnSiguiente_clicked();

private:
    Ui::FrmAlbaranProveedor *ui;
    Table_Helper helper;
    Proveedor prov;
};

#endif // FRMALBARANPROVEEDOR_H
