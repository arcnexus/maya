#ifndef FRMPEDIDOSPROVEEDOR_H
#define FRMPEDIDOSPROVEEDOR_H

#include <QDialog>
#include "../Auxiliares/Globlal_Include.h"
#include "../Auxiliares/table_helper.h"
#include "proveedor.h"
#include "pedidoproveedor.h"

namespace Ui {
class FrmPedidosProveedor;
}

class FrmPedidosProveedor : public QDialog
{
    Q_OBJECT
    
public:
    explicit FrmPedidosProveedor(QWidget *parent = 0);
    ~FrmPedidosProveedor();
    Proveedor *oProveedor = new Proveedor(this);
    PedidoProveedor *oPedido_proveedor = new PedidoProveedor(this);

private slots:
    void totalChanged(double base , double dto ,double subTotal , double iva, double re, double total, QString moneda);
    void desglose1Changed(double base, double iva, double re, double total);
    void desglose2Changed(double base, double iva, double re, double total);
    void desglose3Changed(double base, double iva, double re, double total);
    void desglose4Changed(double base, double iva, double re, double total);
    void estadoedicion();
    void estadolectura();
    void buscar_proveeedor();
    void anadir_pedido();
    void guardar_pedido();
    void llenar_campos();
    void clear();
private:
    Ui::FrmPedidosProveedor *ui;
    Table_Helper helper;
};

#endif // FRMPEDIDOSPROVEEDOR_H
