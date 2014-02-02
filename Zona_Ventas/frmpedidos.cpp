#include "frmpedidos.h"
#include "ui_frmpedidos.h"


#include "../Almacen/articulo.h"
#include "../Zona_Ventas/albaran.h"

#include "../Busquedas/db_consulta_view.h"
#include "../Zona_Ventas/factura.h"
#include"../Auxiliares/datedelegate.h"
#include"../Auxiliares/monetarydelegate.h"
#include "../Auxiliares/monetarydelegate_totals.h"
#include "../Auxiliares/numericdelegate.h"
#include"../Zona_Maestros/transportistas.h"
#include "../vencimientos.h"
#include "../Auxiliares/frmeditline.h"

FrmPedidos::FrmPedidos(QWidget *parent) :
    MayaModule(module_zone(),module_name(),parent),
    ui(new Ui::frmPedidos),
    menuButton(QIcon(":/Icons/PNG/pedidos_cli.png"),tr("Pedidos"),this),
    push(new QPushButton(QIcon(":/Icons/PNG/pedidos_cli.png"),"",this))
{
    ui->setupUi(this);
    ui->tabWidget_2->setCurrentIndex(0);
    ui->cboPais->setModel(Configuracion_global->paises_model);
    ui->cboPais->setModelColumn(1);
    ui->cbopais_entrega->setModel(Configuracion_global->paises_model);
    ui->cbopais_entrega->setModelColumn(1);
    ui->cboPais->setCurrentIndex(-1);
    ui->cbopais_entrega->setCurrentIndex(-1);
    // Pongo valores por defecto
    ui->lblfacturado->setVisible(false);
    ui->lblimpreso->setVisible(false);

    push->setStyleSheet("background-color: rgb(133, 170, 142)");
    push->setToolTip(tr("Gestión de pedidos de clientes"));


    oPedido = new Pedidos();
    oCliente3 = new Cliente();
    // ------------------------------
    // Modelo y formato tabla lineas
    //-------------------------------
    modelLineas = new QSqlQueryModel(this);
    modelLineas->setQuery("select id,codigo,descripcion,cantidad,precio_recom,porc_dto,precio ,subtotal,porc_iva,total "
                          "from lin_ped where id = 0;",Configuracion_global->empresaDB);
    ui->Lineas->setModel(modelLineas);
    QStringList header;
    QVariantList sizes;
    header << tr("id") << tr("Código") << tr("Descripción") << tr("cantidad") << tr("pvp recom")<< tr("porc_dto") << tr("pvp.") << tr("Subtotal");
    header  << tr("porc_iva") << tr("Total");
    sizes << 0 << 100 << 300 << 100 << 100 <<100 << 100 <<100 << 100 <<110;
    for(int i = 0; i <header.size();i++)
    {
        ui->Lineas->setColumnWidth(i,sizes.at(i).toInt());
        modelLineas->setHeaderData(i,Qt::Horizontal,header.at(i));
    }
    ui->Lineas->setItemDelegateForColumn(3,new NumericDelegate(this,true));
    ui->Lineas->setItemDelegateForColumn(4,new MonetaryDelegate(this,true));
    ui->Lineas->setItemDelegateForColumn(5,new MonetaryDelegate(this,true));
    ui->Lineas->setItemDelegateForColumn(6,new MonetaryDelegate(this,true));
    ui->Lineas->setItemDelegateForColumn(7,new MonetaryDelegate(this,true));
    ui->Lineas->setItemDelegateForColumn(8,new MonetaryDelegate(this,true));
    ui->Lineas->setItemDelegateForColumn(9,new MonetaryDelegate_totals(this,true));
    //-------------------------------------------------------


    aAlbaran_action = new QAction(tr("En albaran"),this);
    aFactura_action = new QAction(tr("En factura"),this);

    connect(aAlbaran_action,SIGNAL(triggered()),this,SLOT(convertir_ealbaran()));
    connect(aFactura_action,SIGNAL(triggered()),this,SLOT(convertir_enFactura()));

    convertir_menu = new QMenu(this);
    convertir_menu->addAction(aAlbaran_action);
    convertir_menu->addAction(aFactura_action);

    ui->btn_convertir->setMenu(convertir_menu);


    ui->txtporc_iva1->setText(Configuracion_global->ivaList.at(0));
    ui->txtporc_iva2->setText(Configuracion_global->ivaList.at(1));
    ui->txtporc_iva3->setText(Configuracion_global->ivaList.at(2));
    ui->txtporc_iva4->setText(Configuracion_global->ivaList.at(3));

    ui->txtporc_rec1->setText(Configuracion_global->reList.at(0));
    ui->txtporc_rec2->setText(Configuracion_global->reList.at(1));
    ui->txtporc_rec3->setText(Configuracion_global->reList.at(2));
    ui->txtporc_rec4->setText(Configuracion_global->reList.at(3));

    //--------------
    // LLenar tabla
    //--------------
    m = new QSqlQueryModel(this);
    m->setQuery("select id, pedido, fecha,total_pedido, cliente from ped_cli where ejercicio ="+
                Configuracion_global->cEjercicio+" order by pedido desc",Configuracion_global->empresaDB);
    ui->tabla->setModel(m);
    formato_tabla();
    connect(ui->tabla->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this,SLOT(on_table_row_changed(QModelIndex,QModelIndex)));
    //ui->tabla->selectRow(0);

    //--------------------
    // %Iva combos gastos
    //--------------------
    QSqlQueryModel *iva = new QSqlQueryModel(this);
    iva->setQuery("select iva from tiposiva",Configuracion_global->groupDB);
    ui->cboporc_iva_gasto1->setModel(iva);
    int index = ui->cboporc_iva_gasto1->findText(Configuracion_global->ivaList.at(0));
    ui->cboporc_iva_gasto1->setCurrentIndex(index);
    ui->cboporc_iva_gasto2->setModel(iva);
    index = ui->cboporc_iva_gasto2->findText(Configuracion_global->ivaList.at(0));
    ui->cboporc_iva_gasto1->setCurrentIndex(index);
    ui->cboporc_iva_gasto3->setModel(iva);
    index = ui->cboporc_iva_gasto3->findText(Configuracion_global->ivaList.at(0));
    ui->cboporc_iva_gasto1->setCurrentIndex(index);

    //-------------------
    // Modo Busqueda
    //-------------------

    ui->stackedWidget->setCurrentIndex(1);


    connect(ui->cboporc_iva_gasto1,SIGNAL(currentIndexChanged(int)),
            this,SLOT(cboporc_iva_gasto1_currentIndexChanged(int)));
    connect(ui->cboporc_iva_gasto2,SIGNAL(currentIndexChanged(int)),
            this,SLOT(cboporc_iva_gasto1_currentIndexChanged(int)));
    connect(ui->cboporc_iva_gasto3,SIGNAL(currentIndexChanged(int)),
            this,SLOT(cboporc_iva_gasto1_currentIndexChanged(int)));
    connect(ui->SpinGastoDist1,SIGNAL(valueChanged(double)),
            this,SLOT(SpinGastoDist1_valueChanged(double)));
    connect(ui->SpinGastoDist2,SIGNAL(valueChanged(double)),
            this,SLOT(SpinGastoDist2_valueChanged(double)));
    connect(ui->SpinGastoDist3,SIGNAL(valueChanged(double)),
            this,SLOT(SpinGastoDist3_valueChanged(double)));

    //----------------------
    // Forma de pago
    //----------------------
    ui->cboFormapago->setModel(Configuracion_global->formapago_model);
    ui->cboFormapago->setModelColumn(2);

    //-----------------------
    // Divisas
    //-----------------------
    ui->cboDivisa->setModel(Configuracion_global->divisas_model);
    ui->cboDivisa->setModelColumn(1);



    //----------------------
    // Control de eventos
    //----------------------
//    ui->txtcodigo_cliente->installEventFilter(this);
//    ui->txtcodigo_transportista->installEventFilter(this);
    QList<QWidget*> l = this->findChildren<QWidget*>();
    QList<QWidget*> ::Iterator it;

    for( it = l.begin() ;it!= l.end();++it )
        (*it)->installEventFilter(this);

    setUpBusqueda();

    BloquearCampos(true);
    ui->tabla->selectRow(0);
}

FrmPedidos::~FrmPedidos()
{
    delete ui;
    delete oPedido;
    delete oCliente3;
}

void FrmPedidos::LLenarCampos()
{
    ui->lblTopcliente->setText(oPedido->cliente);
    ui->lblToppedido->setText(QString::number(oPedido->pedido));
    ui->txtalbaran->setText(QString::number(oPedido->albaran));
    ui->txtpedido->setText(QString::number(oPedido->pedido));
    ui->txtfecha->setDate(oPedido->fecha);
    //oPedido->pedido;
    //oPedido->id_cliente;
    ui->txtcodigo_cliente->setText(oPedido->codigo_cliente);
    ui->txtcliente->setText(oPedido->cliente);
    ui->txtdireccion1->setText(oPedido->direccion1);
    ui->txtdireccion2->setText(oPedido->direccion2);
    ui->txtpoblacion->setText(oPedido->poblacion);
    ui->txtprovincia->setText(oPedido->provincia);
    ui->txtcp->setText(oPedido->cp);

    for(int i =0;i<Configuracion_global->paises_model->rowCount();i++)
    {
        if(Configuracion_global->paises_model->record(i).value("id").toInt() == oPedido->id_pais)
        {
            int ind_pais = ui->cboPais->findText(Configuracion_global->paises_model->record(i).value("pais").toString());
            ui->cboPais->setCurrentIndex(ind_pais);
            break;
        }
        else
        {
            ui->cboPais->setCurrentIndex(-1);
        }
    }
    ui->txtTarifa_cliente->setText(Configuracion_global->Devolver_tarifa(oPedido->id_tarifa));
    ui->txtcif->setText(oPedido->cif);
    ui->chkrecargo_equivalencia->setChecked(oPedido->recargo_equivalencia==1);
        //helper.set_tarifa(oCliente3->tarifa_cliente);
    ui->txtsubtotal->setText(QString::number(oPedido->subtotal));
    ui->spin_porc_dto_especial->setValue(oPedido->porc_dto);
    ui->spinPorc_dto_pp->setValue(oPedido->porc_dto_pp);
    ui->txtdto->setText(QString::number(oPedido->dto));
    //oPedido->dto;
    ui->txtbase1->setText(QString::number(oPedido->base1));
    ui->txtbase2->setText(QString::number(oPedido->base2));
    ui->txtbase3->setText(QString::number(oPedido->base3));
    ui->txtbase4->setText(QString::number(oPedido->base4));
    ui->txtporc_iva1->setText(QString::number(oPedido->porc_iva1));
    ui->txtporc_iva2->setText(QString::number(oPedido->porc_iva2));
    ui->txtporc_iva3->setText(QString::number(oPedido->porc_iva3));
    ui->txtporc_iva4->setText(QString::number(oPedido->porc_iva4));
    ui->txtiva1->setText(QString::number(oPedido->iva1));
    ui->txtiva2->setText(QString::number(oPedido->iva2));
    ui->txtiva3->setText(QString::number(oPedido->iva3));
    ui->txtiva4->setText(QString::number(oPedido->iva4));
    ui->txtporc_rec1->setText(QString::number(oPedido->porc_rec1));
    ui->txtporc_rec2->setText(QString::number(oPedido->porc_rec2));
    ui->txtporc_rec3->setText(QString::number(oPedido->porc_rec3));
    ui->txtporc_rec4->setText(QString::number(oPedido->porc_rec4));
    ui->txtrec1->setText(QString::number(oPedido->rec1));
    ui->txtrec2->setText(QString::number(oPedido->rec2));
    ui->txtrec3->setText(QString::number(oPedido->rec3));
    ui->txtrec4->setText(QString::number(oPedido->rec4));
    ui->txttotal1->setText(QString::number(oPedido->total1));
    ui->txttotal2->setText(QString::number(oPedido->total2));
    ui->txttotal3->setText(QString::number(oPedido->total3));
    ui->txttotal4->setText(QString::number(oPedido->total4));
    ui->txtbase_total_2->setText(QString::number(oPedido->base_total));
    ui->txttotal_iva_2->setText(QString::number(oPedido->iva_total));
    ui->txttotal_recargo_2->setText(QString::number(oPedido->rec_total));
    ui->txttotal_2->setText(QString::number(oPedido->total_albaran));
    ui->txtGastoDist1->setText(oPedido->gasto1);
    ui->txtGastoDist2->setText(oPedido->gasto2);
    ui->txtGastoDist3->setText(oPedido->gasto3);
    ui->SpinGastoDist1->setValue(oPedido->imp_gasto1);
    ui->SpinGastoDist2->setValue(oPedido->imp_gasto2);
    ui->SpinGastoDist3->setValue(oPedido->imp_gasto3);
    ui->spiniva_gasto1->setValue(oPedido->iva_gasto1);
    ui->spiniva_gasto2->setValue(oPedido->iva_gasto2);
    ui->spiniva_gasto3->setValue(oPedido->iva_gasto2);
    int index = ui->cboporc_iva_gasto1->findText(QString::number(oPedido->porc_iva_gasto1));
    ui->cboporc_iva_gasto1->setCurrentIndex(index);
    index = ui->cboporc_iva_gasto2->findText(QString::number(oPedido->porc_iva_gasto2));
    ui->cboporc_iva_gasto1->setCurrentIndex(index);
    index = ui->cboporc_iva_gasto3->findText(QString::number(oPedido->porc_iva_gasto3));
    ui->cboporc_iva_gasto1->setCurrentIndex(index);
    ui->lblimpreso->setVisible(oPedido->impreso);

    ui->lblfacturado->setVisible(oPedido->facturado);
    ui->txtcNumFra->setText(oPedido->factura);
    ui->txtfecha_factura->setDate(oPedido->fecha_factura);
    for(int i =0;i<Configuracion_global->formapago_model->rowCount();i++)
    {
        if(Configuracion_global->formapago_model->record(i).value("id").toInt() == oPedido->id_forma_pago)
        {
            int ind_fp = ui->cboFormapago->findText(Configuracion_global->formapago_model->record(i).value("forma_pago").toString());
            ui->cboFormapago->setCurrentIndex(ind_fp);
            break;
        }
        else
        {
            ui->cboFormapago->setCurrentIndex(-1);
        }
    }


    ui->txtcomentario->setText(oPedido->comentario);
    ui->txtentregado_a_cuenta->setText(QString::number(oPedido->entregado_a_cuenta));
    //oPedido->traspasado_albaran;
    //oPedido->traspasado_factura;
    ui->txtdireccion1_entrega->setText(oPedido->direccion_entrega1);
    ui->txtdireccion2_entrega->setText(oPedido->direccion_entrega2);
    ui->txtcp_entrega->setText(oPedido->cp_entrega);
    ui->txtpoblacion_entrega->setText(oPedido->poblacion_entrega);
    ui->txtprovincia_entrega->setText(oPedido->provincia_entrega);
    for(int i =0;i<Configuracion_global->paises_model->rowCount();i++)
    {
        if(Configuracion_global->paises_model->record(i).value("id").toInt() == oPedido->id_pais)
        {
            int ind_pais = ui->cbopais_entrega->findText(Configuracion_global->paises_model->record(i).value("pais").toString());
            ui->cbopais_entrega->setCurrentIndex(ind_pais);
            break;
        }
        else
        {
            ui->cbopais_entrega->setCurrentIndex(-1);
        }
    }
    ui->txtemail_alternativo->setText(oPedido->email_entrega);
    ui->txtcomentarios_alternativo->setText(oPedido->comentarios_entrega);
    ui->chkenviado->setChecked(oPedido->enviado==1);
    ui->chkcompleto->setChecked(oPedido->completo==1);
    ui->chkentregado->setChecked(oPedido->entregado==1);
    ui->txtfecha_limite_entrega->setDate(oPedido->fecha_limite_entrega);
    ui->txttotal->setText(Configuracion_global->toFormatoMoneda( QString::number(oPedido->total_pedido)));
    oCliente3->Recuperar("Select * from clientes where id ="+QString::number(oPedido->id_cliente));

    //----------------
    // Transportista
    //----------------
    transportistas oTrans(this);
    QStringList condiciones,extras;
    condiciones << QString("id=%1").arg(oPedido->id_transportista);
    extras << "";
    oTrans.recuperar(condiciones,extras);
    ui->txtcodigo_transportista->setText(oTrans.h_transportista.value("codigo").toString());
    ui->txtTransportista->setText(oTrans.h_transportista.value("transportista").toString());
    //-------------------
    // divisas
    //-------------------
    for(int i =0;i<Configuracion_global->divisas_model->rowCount();i++)
    {
        if(Configuracion_global->divisas_model->record(i).value("id").toInt() == oPedido->id_divisa)
        {
            int ind_divisa = ui->cboDivisa->findText(Configuracion_global->divisas_model->record(i).value("moneda").toString());
            ui->cboDivisa->setCurrentIndex(ind_divisa);
            break;
        }
        else
        {
            ui->cboDivisa->setCurrentIndex(-1);
        }
    }



    if(oPedido->traspasado_factura)
        ui->btn_convertir->setEnabled(false);

}

void FrmPedidos::LLenarCamposCliente()
{

    oPedido->id_cliente = oCliente3->id;
    ui->lblTopcliente->setText(oCliente3->nombre_fiscal);
    ui->txtcodigo_cliente->setText(oCliente3->codigo_cliente);
    ui->txtcliente->setText(oCliente3->nombre_fiscal);
    ui->txtdireccion1->setText(oCliente3->direccion1);
    ui->txtdireccion2->setText(oCliente3->direccion2);
    ui->txtcp->setText(oCliente3->cp);
    ui->txtpoblacion->setText(oCliente3->poblacion);
    ui->txtprovincia->setText(oCliente3->provincia);
    for(int i =0;i<Configuracion_global->paises_model->rowCount();i++)
    {
        if(Configuracion_global->paises_model->record(i).value("id").toInt() == oCliente3->id_pais)
        {
            int ind_pais = ui->cboPais->findText(Configuracion_global->paises_model->record(i).value("pais").toString());
            ui->cboPais->setCurrentIndex(ind_pais);
            break;
        }
        else
        {
            ui->cboPais->setCurrentIndex(-1);
        }
    }

    ui->txtcif->setText(oCliente3->cif_nif);


    if (oCliente3->recargo_equivalencia==1) {
        ui->chkrecargo_equivalencia->setChecked(true);
        oPedido->recargo_equivalencia = (1);
        ui->txtporc_rec1->setText(Configuracion_global->reList.at(0));
        ui->txtporc_rec2->setText(Configuracion_global->reList.at(1));
        ui->txtporc_rec3->setText(Configuracion_global->reList.at(2));
        ui->txtporc_rec4->setText(Configuracion_global->reList.at(3));
        oPedido->porc_rec1 = ui->txtporc_rec1->text().toFloat();
        oPedido->porc_rec2 = ui->txtporc_rec2->text().toFloat();
        oPedido->porc_rec3 = ui->txtporc_rec3->text().toFloat();
        oPedido->porc_rec4 = ui->txtporc_rec4->text().toFloat();
    } else {
        ui->chkrecargo_equivalencia->setChecked(false);
        oPedido->recargo_equivalencia = (0);
        ui->txtporc_rec1->setText("0.00");
        ui->txtporc_rec2->setText("0.00");
        ui->txtporc_rec3->setText("0.00");
        ui->txtporc_rec4->setText("0.00");
        oPedido->porc_rec1 = 0;
        oPedido->porc_rec2 = 0;
        oPedido->porc_rec3 = 0;
        oPedido->porc_rec4 = 0;

    }
    QString error;
    ui->txtTarifa_cliente->setText(SqlCalls::SelectOneField("codigotarifa","descripcion",QString("id=%1").arg(oCliente3->idTarifa),
                                                            Configuracion_global->groupDB,error).toString());
    oPedido->id_tarifa = oCliente3->idTarifa;

    oCliente3->Recuperar("Select * from clientes where id ="+QString::number(oPedido->id_cliente));


    //---------------------------------
    // Comprobar direccion alternativa
    //---------------------------------
    QMap <int,QSqlRecord> rec;
    QStringList condiciones;
    condiciones << QString("id_cliente = %1").arg(oCliente3->id) << "direccion_envio = 1";
    rec = SqlCalls::SelectRecord("cliente_direcciones",condiciones,Configuracion_global->groupDB,error);

    QMapIterator <int,QSqlRecord> i(rec);
    while(i.hasNext())
    {
        i.next();
        ui->txtdireccion1_entrega->setText(i.value().value("direccion1").toString());
        ui->txtdireccion2_entrega->setText(i.value().value("direccion2").toString());
        ui->txtcp_entrega->setText(i.value().value("cp").toString());
        ui->txtpoblacion_entrega->setText(i.value().value("poblacion").toString());
        ui->txtprovincia_entrega->setText(i.value().value("provincia").toString());
        for(int i =0;i<Configuracion_global->paises_model->rowCount();i++)
        {
            if(Configuracion_global->paises_model->record(i).value("id").toInt() == oCliente3->id_pais_alternativa)
            {
                int ind_pais = ui->cbopais_entrega->findText(Configuracion_global->paises_model->record(i).value("pais").toString());
                ui->cbopais_entrega->setCurrentIndex(ind_pais);
                break;
            }
            else
            {
                ui->cbopais_entrega->setCurrentIndex(-1);
            }
        }
        ui->txtemail_alternativo->setText(i.value().value("email").toString());
        ui->txtcomentarios_alternativo->setPlainText(i.value().value("comentarios").toString());

    }
    if(Configuracion_global->formapago_model->rowCount() >0)
    {
        for(int i =0;i<Configuracion_global->formapago_model->rowCount();i++)
        {
            if(Configuracion_global->formapago_model->record(i).value("id").toInt() == oCliente3->id_forma_pago)
            {
                int ind_fp = ui->cboFormapago->findText(Configuracion_global->formapago_model->record(i).value(
                                                             "forma_pago").toString());
                ui->cboFormapago->setCurrentIndex(ind_fp);
                break;
            }
            else
            {
                ui->cboFormapago->setCurrentIndex(-1);
            }

        }
    }
    //-------------------
    // divisas
    //-------------------
    for(int i =0;i<Configuracion_global->divisas_model->rowCount();i++)
    {
        if(Configuracion_global->divisas_model->record(i).value("id").toInt() == oCliente3->id_divisa)
        {
            int ind_divisa = ui->cboDivisa->findText(Configuracion_global->divisas_model->record(i).value("moneda").toString());
            ui->cboDivisa->setCurrentIndex(ind_divisa);
            break;
        }
        else
        {
            ui->cboDivisa->setCurrentIndex(-1);
        }
    }

}

void FrmPedidos::VaciarCampos()
{
    ui->lblTopcliente->setText("");
    ui->lblToppedido->setText("");
    QDate fecha;
    ui->txtcodigo_cliente->setText("");
    ui->txtpedido->setText("");
    ui->txtfecha->setDate(fecha.currentDate());
    ui->txtfecha_factura->setDate(fecha.currentDate());
    ui->txtcliente->setText("");
    ui->txtdireccion1->setText("");
    ui->txtdireccion2->setText("");
    ui->txtcp->setText("");
    ui->txtpoblacion->setText("");
    ui->txtprovincia->setText("");
    ui->cboPais->setCurrentIndex(-1);
    ui->cboFormapago->setCurrentIndex(-1);
    ui->cboDivisa->setCurrentIndex(-1);
    ui->cbopais_entrega->setCurrentIndex(-1);
    ui->txtcif->setText("");
    ui->txtsubtotal->setText(0);
    ui->txtdto->setText("0,00");
    ui->txtbase->setText("0,00");
    ui->txtiva->setText("0,00");
    ui->txttotal->setText("0,00");
    ui->lblimpreso->setVisible(false);
    ui->lblfacturado->setVisible(false);
    ui->txtcomentario->setText("");
    ui->txtbase1->setText(0);
    ui->txtbase2->setText(0);
    ui->txtbase3->setText(0);
    ui->txtbase4->setText(0);
    ui->txtporc_iva1->setText(QString::number(Configuracion_global->ivaList.at(0).toDouble()));
    ui->txtporc_iva2->setText(QString::number(Configuracion_global->ivaList.at(1).toDouble()));
    ui->txtporc_iva3->setText(QString::number(Configuracion_global->ivaList.at(2).toDouble()));
    ui->txtporc_iva4->setText(QString::number(Configuracion_global->ivaList.at(3).toDouble()));
    ui->txtporc_rec1->setText("0.00");
    ui->txtporc_rec2->setText("0.00");
    ui->txtporc_rec3->setText("0.00");
    ui->txtporc_rec4->setText("0.00");
    ui->txtiva1->setText(0);
    ui->txtiva2->setText(0);
    ui->txtiva3->setText(0);
    ui->txtiva4->setText(0);
    ui->txttotal1->setText(0);
    ui->txttotal2->setText(0);
    ui->txttotal3->setText(0);
    ui->txttotal4->setText(0);
    ui->txtporc_rec1->setText(0);
    ui->txtporc_rec2->setText(0);
    ui->txtporc_rec3->setText(0);
    ui->txtporc_rec4->setText(0);
    ui->txttotal_recargo_2->setText(0);
    ui->txtrec->setText("0,00");
    ui->txtentregado_a_cuenta->setText("0,00");
    ui->txtpedido_cliente->setText("");
    ui->txttotal_iva_2->setText("0,00");
    ui->txtbase_total_2->setText("0,00");
    ui->txttotal_recargo_2->setText("0,00");
    ui->txttotal_2->setText("0,00");
    ui->txtsubtotal->setText("0,00");
    ui->txtpedido->setReadOnly(true);
    ui->txtGastoDist1->clear();
    ui->txtGastoDist2->clear();
    ui->txtGastoDist3->clear();
    ui->SpinGastoDist1->clear();
    ui->SpinGastoDist2->clear();
    ui->SpinGastoDist3->clear();
    ui->txtcNumFra->clear();
    ui->spiniva_gasto1->clear();
    ui->spiniva_gasto2->clear();
    ui->spiniva_gasto3->clear();
    this->LLenarPedido();

    modelLineas->setQuery(QString("select id,codigo,descripcion,cantidad,precio,precio_recom,subtotal,porc_dto,porc_iva,total "
                              "from lin_ped where id_cab = 0 limit 0;"),Configuracion_global->empresaDB);

 }


void FrmPedidos::LLenarPedido()
{
    oPedido->albaran= ui->txtalbaran->text().toInt();
    oPedido->pedido=ui->txtpedido->text().toInt();
    oPedido->fecha=ui->txtfecha->date();
    oPedido->id_divisa = Configuracion_global->Devolver_id_moneda(ui->cboDivisa->currentText());
    oPedido->codigo_cliente=ui->txtcodigo_cliente->text();
    oPedido->cliente=ui->txtcliente->text();
    oPedido->direccion1=ui->txtdireccion1->text();
    oPedido->direccion2=ui->txtdireccion2->text();
    oPedido->poblacion=ui->txtpoblacion->text();
    oPedido->provincia=ui->txtprovincia->text();
    oPedido->cp=ui->txtcp->text();
    oPedido->id_pais = Configuracion_global->Devolver_id_pais(ui->cboPais->currentText());
    oPedido->cif=ui->txtcif->text();
    oPedido->recargo_equivalencia=ui->chkrecargo_equivalencia->isChecked();
    oPedido->subtotal=ui->txtsubtotal->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->dto=ui->txtdto->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->porc_dto = ui->spin_porc_dto_especial->value();
    oPedido->porc_dto_pp = ui->spinPorc_dto_pp->value();
    oPedido->base1=ui->txtbase1->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->base2=ui->txtbase2->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->base3=ui->txtbase3->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->base4=ui->txtbase4->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->porc_iva1=ui->txtporc_iva1->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();

    oPedido->porc_iva2=ui->txtporc_iva2->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->porc_iva3=ui->txtporc_iva3->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->porc_iva4=ui->txtporc_iva4->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->iva1=ui->txtiva1->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->iva2=ui->txtiva2->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->iva3=ui->txtiva3->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->iva4=ui->txtiva4->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->porc_rec1=ui->txtporc_rec1->text().toDouble();
    oPedido->porc_rec2=ui->txtporc_rec2->text().toDouble();
    oPedido->porc_rec3=ui->txtporc_rec3->text().toDouble();
    oPedido->porc_rec4=ui->txtporc_rec4->text().toDouble();
    oPedido->rec1=ui->txtporc_rec1->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->rec2=ui->txtporc_rec2->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->rec3=ui->txtporc_rec3->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->rec4=ui->txtporc_rec4->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->total1=ui->txttotal1->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->total2=ui->txttotal2->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->total3=ui->txttotal3->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->total4=ui->txttotal4->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->base_total=ui->txtbase_total_2->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->iva_total=ui->txttotal_iva_2->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->rec_total=ui->txttotal_recargo_2->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->total_albaran=ui->txttotal_2->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();

    oPedido->impreso = ui->lblimpreso->isVisible();

    oPedido->fecha_factura=ui->txtfecha_factura->date();

    oPedido->comentario=ui->txtcomentario->toPlainText();
    oPedido->entregado_a_cuenta=ui->txtentregado_a_cuenta->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->direccion_entrega1 = ui->txtdireccion1_entrega->text();
    oPedido->direccion_entrega2 = ui->txtdireccion2_entrega->text();
    oPedido->cp_entrega=ui->txtcp_entrega->text();
    oPedido->poblacion_entrega=ui->txtpoblacion_entrega->text();
    oPedido->provincia_entrega=ui->txtprovincia_entrega->text();
    oPedido->id_pais_entrega=Configuracion_global->Devolver_id_pais(ui->cbopais_entrega->currentText());
    oPedido->email_entrega = ui->txtemail_alternativo->text();
    oPedido->comentarios_entrega = ui->txtcomentarios_alternativo->toPlainText();
    oPedido->enviado=ui->chkenviado->isChecked();
    oPedido->completo=ui->chkcompleto->isChecked();
    oPedido->entregado=ui->chkentregado->isChecked();
    oPedido->fecha_limite_entrega=ui->txtfecha_limite_entrega->date();
    oPedido->total_pedido=ui->txttotal->text().replace(_moneda,"").replace(".","").replace(",",".").toDouble();
    oPedido->gasto1 = ui->txtGastoDist1->text();
    oPedido->gasto2 = ui->txtGastoDist2->text();
    oPedido->gasto3 = ui->txtGastoDist3->text();
    oPedido->imp_gasto1 = ui->SpinGastoDist1->value();
    oPedido->imp_gasto2 = ui->SpinGastoDist2->value();
    oPedido->imp_gasto3 = ui->SpinGastoDist3->value();
    oPedido->porc_iva_gasto1 = ui->cboporc_iva_gasto1->currentText().toFloat();
    oPedido->porc_iva_gasto2 = ui->cboporc_iva_gasto2->currentText().toFloat();
    oPedido->porc_iva_gasto3 = ui->cboporc_iva_gasto3->currentText().toFloat();
    oPedido->iva_gasto1 = ui->spiniva_gasto1->value();
    oPedido->iva_gasto2 = ui->spiniva_gasto2->value();
    oPedido->iva_gasto3 = ui->spiniva_gasto3->value();
    oPedido->id_forma_pago = Configuracion_global->Devolver_id_forma_pago(ui->cboFormapago->currentText());
}

void FrmPedidos::formato_tabla()
{
    // id, pedido, fecha, cliente
    QStringList headers;
    QVariantList size;
    headers << "id" <<tr("Pedido") << tr("fecha") <<tr("Total") <<tr("cliente");
    size <<0 <<120 <<120 <<120 <<300;
    for(int i = 0;i<headers.length();i++)
    {
        ui->tabla->setColumnWidth(i,size.at(i).toInt());
        m->setHeaderData(i,Qt::Horizontal,headers.at(i));
    }
    ui->tabla->setItemDelegateForColumn(2,new DateDelegate(this));
    ui->tabla->setItemDelegateForColumn(3, new MonetaryDelegate(this));
    ui->tabla->setColumnHidden(0,true);
}

void FrmPedidos::filter_table(QString texto, QString orden, QString modo)
{
    this->texto = texto;
    this->orden = orden;
    this->modo = modo;
    ui->stackedWidget->setCurrentIndex(1);
    //  tr("Pedido") <<tr("Cliente") <<tr("Fecha")
    QHash <QString,QString> h;
    h[tr("Pedido")] = "pedido";
    h[tr("Cliente")] = "cliente";
    h[tr("Fecha")] = "fecha";
    h[tr("Total")] = "total_pedido";
    QString order = h.value(orden);

    if (modo == tr("A-Z"))
        modo = "";
    else
        modo = "DESC";
    QString cSQL = "select id, pedido, fecha,total_pedido, cliente from ped_cli where "+order+" like '%" + texto.trimmed() +
            "%' and ejercicio ="+
            Configuracion_global->cEjercicio +" order by "+order+" "+modo;
    m->setQuery(cSQL,Configuracion_global->empresaDB);

    ui->tabla->selectRow(0);
}

void FrmPedidos::on_table_row_changed(QModelIndex actual, QModelIndex previous)
{
    Q_UNUSED(previous);
    on_tabla_clicked(actual);
}

void FrmPedidos::calcular_iva_gastos()
{
    calcular_pedido();
}

void FrmPedidos::buscar_transportista()
{
    db_consulta_view consulta;
    QStringList campos;
    campos <<"codigo" <<"transportista";
    consulta.set_campoBusqueda(campos);
    consulta.set_texto_tabla("transportista");
    consulta.set_db("Maya");
    consulta.set_SQL("select id,codigo,transportista from transportista");
    QStringList cabecera;
    QVariantList tamanos;
    cabecera  << tr("Código") << tr("Nombre");
    tamanos <<"0" << "100" << "300";
    consulta.set_headers(cabecera);
    consulta.set_tamano_columnas(tamanos);
    consulta.set_titulo("Busqueda de Transportistas");
    if(consulta.exec())
    {
        int id = consulta.get_id();
        transportistas oTrans(this);
        QStringList condiciones,extras;
        condiciones <<QString("id=%1").arg(id);
        extras << "";
        oTrans.recuperar(condiciones,extras);
        ui->txtcodigo_transportista->setText(oTrans.h_transportista.value("codigo").toString());
        ui->txtTransportista->setText(oTrans.h_transportista.value("transportista").toString());
        oPedido->id_transportista = oTrans.h_transportista.value("id").toInt();
    }
}

bool FrmPedidos::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if(obj == ui->Lineas){
            if (keyEvent->key() == Qt::Key_Plus)
            {
                on_btnAnadirLinea_clicked();
            }
            if(keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
                on_Lineas_doubleClicked(ui->Lineas->currentIndex());

        }
        if(obj == ui->txtcodigo_transportista)
        {
            if(keyEvent->key() == Qt::Key_F1)
                buscar_transportista();
        }
        if(obj == ui->txtcodigo_cliente)
        {
            if(keyEvent->key() == Qt::Key_F1)
                on_botBuscarCliente_clicked();
        }
        if(keyEvent->key() == Qt::Key_F1)
        {
            if(ui->btnEditar->isEnabled())
            {
                if(m_busqueda->isShow())
                    ocultarBusqueda();
                else
                    mostrarBusqueda();
            }
            return true;
        }
        if(keyEvent->key() == Qt::Key_Return)
            if(ui->stackedWidget->currentIndex() == 1)
                on_tabla_doubleClicked(ui->tabla->currentIndex());
        if(keyEvent->key() == Qt::Key_Escape)
            return true;
    }
    if(event->type() == QEvent::Resize)
        _resizeBarraBusqueda(m_busqueda);
    return MayaModule::eventFilter(obj,event);
}

void FrmPedidos::calcular_pedido()
{
    double subtotal,dto,dtopp,base,irpf,iva,re,total;
    double base1,iva1,re1,total1;
    double base2,iva2,re2,total2;
    double base3,iva3,re3,total3;
    double base4,iva4,re4,total4;
    base1= 0; base2 = 0; base3 =0;base4 = 0;
    iva1=0; iva2 =0; iva3=0; iva4 = 0;
    re1=0;re2=0;re3=0;re4 =0;
    total1 =0; total2 =0;total3=0; total4=0;
    subtotal =0; dto=0; dtopp =0; base =0; irpf = 0; iva =0; re =0; total =0;

    ui->spiniva_gasto1->setValue((ui->SpinGastoDist1->value()*(ui->cboporc_iva_gasto1->currentText().toDouble()/100)));
    ui->spiniva_gasto2->setValue((ui->SpinGastoDist2->value()*(ui->cboporc_iva_gasto2->currentText().toDouble()/100)));
    ui->spiniva_gasto3->setValue((ui->SpinGastoDist3->value()*(ui->cboporc_iva_gasto3->currentText().toDouble()/100)));

    QMap <int,QSqlRecord> l;
    QString error;
    l = SqlCalls::SelectRecord("lin_ped",QString("id_cab=%1").arg(oPedido->id),Configuracion_global->empresaDB,error);
    QMapIterator <int,QSqlRecord> li(l);
    while(li.hasNext())
    {
        li.next();
        subtotal += li.value().value("subtotal").toDouble();
        if(li.value().value("porc_iva").toFloat() == ui->txtporc_iva1->text().toFloat())
        {
            // base1
            base1 += li.value().value("total").toDouble();
            iva1 = base1*(li.value().value("porc_iva").toFloat()/100);
            re1 = base1*(li.value().value("porc_re").toFloat()/100);
            total1 = base1 + iva1 + re1;
        }
        // base2
        if(li.value().value("porc_iva").toFloat() == ui->txtporc_iva2->text().toFloat())
        {
            base2 += li.value().value("total").toDouble();
            iva2 = base2*(li.value().value("porc_iva").toFloat()/100);
            re2 = base2*(li.value().value("porc_re").toFloat()/100);
            total2 = base2 + iva2 + re2;

        }
        // base3
        if(li.value().value("porc_iva").toFloat() == ui->txtporc_iva3->text().toFloat())
        {
            base3 += li.value().value("total").toDouble();
            iva3 = base3*(li.value().value("porc_iva").toFloat()/100);
            re3 = base3*(li.value().value("porc_re").toFloat()/100);
            total3 = base3 + iva3 + re3;

        }
        // base4
        if(li.value().value("porc_iva").toFloat() == ui->txtporc_iva4->text().toFloat())
        {
            base4 += li.value().value("total").toDouble();
            iva4 = base4*(li.value().value("porc_iva").toFloat()/100);
            re4 = base4*(li.value().value("porc_re").toFloat()/100);
            total4 = base4 + iva4 + re4;
        }

    }
    // añadir gastos extras
    if(ui->cboporc_iva_gasto1->currentText().toFloat() == ui->txtporc_iva1->text().toFloat())
        base1 += ui->SpinGastoDist1->value();
    if(ui->cboporc_iva_gasto2->currentText().toFloat() == ui->txtporc_iva1->text().toFloat())
        base1 += ui->SpinGastoDist2->value();
    if(ui->cboporc_iva_gasto3->currentText().toFloat() == ui->txtporc_iva1->text().toFloat())
        base1 += ui->SpinGastoDist3->value();
    iva1 = base1 * (ui->txtporc_iva1->text().toFloat()/100);
    if(ui->chkrecargo_equivalencia->isChecked())
        re1 = base1 * (ui->txtrec1->text().toFloat()/100);

    if(ui->cboporc_iva_gasto1->currentText().toFloat() == ui->txtporc_iva2->text().toFloat())
        base2 += ui->SpinGastoDist1->value();
    if(ui->cboporc_iva_gasto2->currentText().toFloat() == ui->txtporc_iva2->text().toFloat())
        base2 += ui->SpinGastoDist2->value();
    if(ui->cboporc_iva_gasto3->currentText().toFloat() == ui->txtporc_iva2->text().toFloat())
        base2 += ui->SpinGastoDist3->value();
    iva2 = base2 * (ui->txtporc_iva2->text().toFloat()/100);
    if(ui->chkrecargo_equivalencia->isChecked())
        re2 = base2 * (ui->txtrec1->text().toFloat()/100);

    if(ui->cboporc_iva_gasto1->currentText().toFloat() == ui->txtporc_iva3->text().toFloat())
        base3 += ui->SpinGastoDist1->value();
    if(ui->cboporc_iva_gasto2->currentText().toFloat() == ui->txtporc_iva3->text().toFloat())
        base3 += ui->SpinGastoDist2->value();
    if(ui->cboporc_iva_gasto3->currentText().toFloat() == ui->txtporc_iva3->text().toFloat())
        base3 += ui->SpinGastoDist3->value();
    iva3 = base3 * (ui->txtporc_iva3->text().toFloat()/100);
    if(ui->chkrecargo_equivalencia->isChecked())
        re3 = base3 * (ui->txtrec3->text().toFloat()/100);

    if(ui->cboporc_iva_gasto1->currentText().toFloat() == ui->txtporc_iva4->text().toFloat())
        base4 += ui->SpinGastoDist1->value();
    if(ui->cboporc_iva_gasto2->currentText().toFloat() == ui->txtporc_iva4->text().toFloat())
        base4 += ui->SpinGastoDist2->value();
    if(ui->cboporc_iva_gasto3->currentText().toFloat() == ui->txtporc_iva4->text().toFloat())
        base4 += ui->SpinGastoDist3->value();
    iva4 = base4 * (ui->txtporc_iva4->text().toFloat()/100);
    if(ui->chkrecargo_equivalencia->isChecked())
        re4 = base4 * (ui->txtrec4->text().toFloat()/100);

    // TOTALES GENERALES

    if(l.size() >0)
        dto += li.value().value("dto").toDouble();
    base = base1 + base2+base3+base4;
    total1 = base1 +iva1 +re1;
    total2 = base2 +iva2 +re2;
    total3 = base3 +iva3 +re3;
    total4 = base4 +iva4 +re4;
    dtopp = subtotal *(ui->spinPorc_dto_pp->value()/100.0);
    base -= dtopp;
    irpf = base *(ui->spinPorc_irpf->value()/100.0);
    ui->txtimporte_irpf->setText(Configuracion_global->toFormatoMoneda(QString::number(irpf,'f',
                                                               Configuracion_global->decimales_campos_totales)));
    ui->txtDto_pp->setText(Configuracion_global->toFormatoMoneda(QString::number(dtopp,'f',
                                                Configuracion_global->decimales_campos_totales)));


    // Desglose llenar controles
    ui->txtbase1->setText(Configuracion_global->toFormatoMoneda(QString::number(base1,'f',
                                                                                Configuracion_global->decimales_campos_totales)));
    ui->txtiva1->setText(Configuracion_global->toFormatoMoneda(QString::number(iva1,'f',
                                                                               Configuracion_global->decimales_campos_totales)));
    ui->txtrec1->setText(Configuracion_global->toFormatoMoneda(QString::number(re1,'f',
                                                                               Configuracion_global->decimales_campos_totales)));
    ui->txttotal1->setText(Configuracion_global->toFormatoMoneda(QString::number(total1,'f',
                                                                 Configuracion_global->decimales_campos_totales)));

    // Desglose llenar controles
    ui->txtbase2->setText(Configuracion_global->toFormatoMoneda(QString::number(base2,'f',
                                                                                Configuracion_global->decimales_campos_totales)));
    ui->txtiva2->setText(Configuracion_global->toFormatoMoneda(QString::number(iva2,'f',
                                                                               Configuracion_global->decimales_campos_totales)));
    ui->txtrec2->setText(Configuracion_global->toFormatoMoneda(QString::number(re2,'f',
                                                                               Configuracion_global->decimales_campos_totales)));
    ui->txttotal2->setText(Configuracion_global->toFormatoMoneda(QString::number(total2,'f',
                                                                 Configuracion_global->decimales_campos_totales)));
    // Desglose llenar controles
    ui->txtbase3->setText(Configuracion_global->toFormatoMoneda(QString::number(base3,'f',
                                                                                Configuracion_global->decimales_campos_totales)));
    ui->txtiva3->setText(Configuracion_global->toFormatoMoneda(QString::number(iva3,'f',
                                                                               Configuracion_global->decimales_campos_totales)));
    ui->txtrec3->setText(Configuracion_global->toFormatoMoneda(QString::number(re3,'f',
                                                                               Configuracion_global->decimales_campos_totales)));
    ui->txttotal3->setText(Configuracion_global->toFormatoMoneda(QString::number(total3,'f',
                                                                 Configuracion_global->decimales_campos_totales)));
    // Desglose llenar controles
    ui->txtbase4->setText(Configuracion_global->toFormatoMoneda(QString::number(base4,'f',
                                                                                Configuracion_global->decimales_campos_totales)));
    ui->txtiva4->setText(Configuracion_global->toFormatoMoneda(QString::number(iva4,'f',
                                                                               Configuracion_global->decimales_campos_totales)));
    ui->txtrec4->setText(Configuracion_global->toFormatoMoneda(QString::number(re4,'f',
                                                                               Configuracion_global->decimales_campos_totales)));
    ui->txttotal4->setText(Configuracion_global->toFormatoMoneda(QString::number(total4,'f',
                                                                 Configuracion_global->decimales_campos_totales)));

    ui->txtdto->setText(Configuracion_global->toFormatoMoneda(Configuracion_global->toRound(dtopp,Configuracion_global->decimales_campos_totales)));
    iva = iva1 + iva2 +iva3+iva4;
    re  = re1 +re2 + re3 + re4;
    total = (base - irpf) + iva +re;

    this->moneda = moneda;
    ui->txtbase->setText(Configuracion_global->toFormatoMoneda(QString::number(base,'f',Configuracion_global->decimales_campos_totales)));
    ui->txtdto->setText(Configuracion_global->toFormatoMoneda(QString::number(dto,'f',Configuracion_global->decimales_campos_totales)));
    ui->txtsubtotal->setText(Configuracion_global->toFormatoMoneda(QString::number(subtotal,'f',Configuracion_global->decimales_campos_totales)));
    ui->txtiva->setText(Configuracion_global->toFormatoMoneda(QString::number(iva,'f',Configuracion_global->decimales_campos_totales)));
    ui->txtrec->setText(Configuracion_global->toFormatoMoneda(QString::number(re,'f',Configuracion_global->decimales_campos_totales)));
    ui->txttotal->setText(Configuracion_global->toFormatoMoneda(QString::number((base-irpf)+(iva+re),'f',Configuracion_global->decimales_campos_totales)));
    ui->txtrec->setText(Configuracion_global->toFormatoMoneda(QString::number(re,'f',Configuracion_global->decimales_campos_totales)));

    ui->txtbase->setText(Configuracion_global->toFormatoMoneda(QString::number(base,'f',Configuracion_global->decimales_campos_totales)));
    ui->txtiva->setText(Configuracion_global->toFormatoMoneda(QString::number(iva,'f',Configuracion_global->decimales_campos_totales)));
    ui->txtrec->setText(Configuracion_global->toFormatoMoneda(QString::number(re,'f',Configuracion_global->decimales_campos_totales)));
    ui->txttotal->setText(Configuracion_global->toFormatoMoneda(QString::number((base-irpf)+(iva+re),'f',Configuracion_global->decimales_campos_totales)));

    ui->txtbase_total_2->setText(Configuracion_global->toFormatoMoneda(QString::number(base,'f',Configuracion_global->decimales_campos_totales)));
    ui->txttotal_iva_2->setText(Configuracion_global->toFormatoMoneda(QString::number(iva,'f',Configuracion_global->decimales_campos_totales)));
    ui->txttotal_recargo_2->setText(Configuracion_global->toFormatoMoneda(QString::number(re,'f',Configuracion_global->decimales_campos_totales)));
    ui->txttotal_2->setText(Configuracion_global->toFormatoMoneda(QString::number((base-irpf)+(iva+re),'f',Configuracion_global->decimales_campos_totales)));


}

void FrmPedidos::setUpBusqueda()
{
    m_busqueda = new BarraBusqueda(this);
    this->setMouseTracking(true);
    this->setAttribute(Qt::WA_Hover);
    this->installEventFilter(this);

    QStringList orden;
    orden  << tr("Pedido") <<tr("Cliente") <<tr("Fecha");
    m_busqueda->setOrderCombo(orden);

    QStringList modo;
    modo << tr("A-Z") << tr("Z-A");
    m_busqueda->setModeCombo(modo);

    connect(m_busqueda,SIGNAL(showMe()),this,SLOT(mostrarBusqueda()));
    connect(m_busqueda,SIGNAL(hideMe()),this,SLOT(ocultarBusqueda()));
    connect(m_busqueda,SIGNAL(doSearch(QString,QString,QString)),this,SLOT(filter_table(QString,QString,QString)));


    QPushButton* add = new QPushButton(QIcon(":/Icons/PNG/add.png"),tr("Añadir"),this);
    connect(add,SIGNAL(clicked()),this,SLOT(on_btnAnadir_clicked()));
    m_busqueda->addWidget(add);

    QPushButton* edit = new QPushButton(QIcon(":/Icons/PNG/edit.png"),tr("Editar"),this);
    connect(edit,SIGNAL(clicked()),this,SLOT(on_btnEditar_clicked()));
    m_busqueda->addWidget(edit);

    QPushButton* print = new QPushButton(QIcon(":/Icons/PNG/print2.png"),tr("Imprimir"),this);
   // connect(print,SIGNAL(clicked()),this,SLOT(on_btnEditar_2_clicked()));//TODO
    m_busqueda->addWidget(print);

    QPushButton* del = new QPushButton(QIcon(":/Icons/PNG/borrar.png"),tr("Borrar"),this);
    connect(del,SIGNAL(clicked()),this,SLOT(on_btn_borrar_clicked()));
    m_busqueda->addWidget(del);

    connect(m_busqueda,SIGNAL(key_Down_Pressed()),ui->tabla,SLOT(setFocus()));
    connect(m_busqueda,SIGNAL(key_F2_Pressed()),this,SLOT(ocultarBusqueda()));
    this->orden = "Pedido";
    this->modo ="A-Z";

}



void FrmPedidos::BloquearCampos(bool state)
{
    //helper.blockTable(state);
    ui->btnAnadirLinea->setEnabled(!state);
    ui->btn_borrarLinea->setEnabled(!state);
    QList<QLineEdit *> lineEditList = this->findChildren<QLineEdit *>();
    QLineEdit *lineEdit;
    foreach (lineEdit, lineEditList) {
        lineEdit->setReadOnly(state);
    }
    // ComboBox
    QList<QComboBox *> ComboBoxList = this->findChildren<QComboBox *>();
    QComboBox *ComboBox;
    foreach (ComboBox, ComboBoxList) {
        ComboBox->setEnabled(!state);
    }
    // DoubleSpinBox
    QList<QDoubleSpinBox *> DblSpinBoxList = this->findChildren<QDoubleSpinBox *>();
    QDoubleSpinBox *DblSpinBox;
    foreach (DblSpinBox, DblSpinBoxList) {
        DblSpinBox->setEnabled(!state);
    }
    // CheckBox
    QList<QCheckBox *> CheckBoxList = this->findChildren<QCheckBox *>();
    QCheckBox *CheckBox;
    foreach (CheckBox, CheckBoxList) {
        CheckBox->setEnabled(!state);
    }
    // QTextEdit
    QList<QTextEdit *> textEditList = this->findChildren<QTextEdit *>();
    QTextEdit *textEdit;
    foreach (textEdit,textEditList) {
        textEdit->setReadOnly(state);
    }
    // QDateEdit
    QList<QDateEdit *> DateEditList = this->findChildren<QDateEdit *>();
    QDateEdit *DateEdit;
    foreach (DateEdit, DateEditList) {
        DateEdit->setEnabled(!state);
    }

    ui->btnAnadir->setEnabled(state);
    ui->btnAnterior->setEnabled(state);
    ui->btnBuscar->setEnabled(state);
    ui->btnDeshacer->setEnabled(!state);
    ui->btnEditar->setEnabled(state);
    ui->btnGuardar->setEnabled(!state);
    ui->btnSiguiente->setEnabled(state);
    ui->botBuscarCliente->setEnabled(!state);
    ui->txtpedido->setReadOnly(true);
    ui->btn_borrar->setEnabled(state);

    ui->chkrecargo_equivalencia->setEnabled(false);
    ui->spiniva_gasto1->setEnabled(false);
    ui->spiniva_gasto2->setEnabled(false);
    ui->spiniva_gasto3->setEnabled(false);
    ui->btn_convertir->setEnabled(state);

    m_busqueda->block(!state);
}
void FrmPedidos::on_btnSiguiente_clicked()
{
    int pedido = oPedido->id;
    if(oPedido->RecuperarPedido("Select * from ped_cli where id >'"+QString::number(pedido)+"' order by pedido  limit 1 "))
    {
        LLenarCampos();
        QString filter = QString("id_Cab = '%1'").arg(oPedido->id);
        //helper.fillTable("empresa","lin_ped",filter);
        ui->btn_borrar->setEnabled(true);
        ui->btnEditar->setEnabled(true);
        ui->btn_convertir->setEnabled(true);
        ui->btnAnterior->setEnabled(true);
        ui->btnImprimir->setEnabled(true);
    }
    else
    {
        TimedMessageBox * t = new TimedMessageBox(this,tr("Se ha llegado al final del archivo.\n"
                                                          "No hay mas presupuestos disponibles"));
        VaciarCampos();
        ui->btn_borrar->setEnabled(false);
        ui->btnEditar->setEnabled(false);
        ui->btn_convertir->setEnabled(false);
        ui->btnSiguiente->setEnabled(false);
        oPedido->id++;
    }
    refrescar_modelo();
}

void FrmPedidos::on_btnAnterior_clicked()
{
    int pedido = oPedido->id;
    if(oPedido->RecuperarPedido("Select * from ped_cli where id <'"+QString::number(pedido)+"' order by pedido desc limit 1 "))
    {
        LLenarCampos();
        QString filter = QString("id_Cab = '%1'").arg(oPedido->id);
        //helper.fillTable("empresa","lin_ped",filter);
        ui->btn_borrar->setEnabled(true);
        ui->btnEditar->setEnabled(true);
        ui->btn_convertir->setEnabled(true);
        ui->btnSiguiente->setEnabled(true);
    }
    else
    {
        TimedMessageBox * t = new TimedMessageBox(this,tr("Se ha llegado al final del archivo.\n"
                                                          "No hay mas presupuestos disponibles"));
        VaciarCampos();
        ui->btn_borrar->setEnabled(false);
        ui->btnEditar->setEnabled(false);
        ui->btn_convertir->setEnabled(false);
        ui->btnAnterior->setEnabled(false);
        oPedido->id = -1;
    }
    refrescar_modelo();
}

void FrmPedidos::on_btnAnadir_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    VaciarCampos();
    BloquearCampos(false);
    ui->txtpedido->setText(QString::number(oPedido->NuevoNumeroPedido()));
    ui->lblToppedido->setText(QString::number(oPedido->NuevoNumeroPedido()));
    int new_id = oPedido->AnadirPedido();
    if (new_id >0)
    {
        oPedido->RecuperarPedido(QString("select * from ped_cli where id =%1").arg(new_id));

        ui->txtcodigo_cliente->setFocus();
        editando = false;
        LLenarCampos();
    } else
    {
        QMessageBox::warning(this,tr("Gestión de pedidos"), tr("Atención: No se pudo crear el pedido"),tr("Aceptar"));
    }
    emit block();
    ocultarBusqueda();

}



void FrmPedidos::on_btnEditar_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    ocultarBusqueda();
    if (oPedido->editable)
    {
        BloquearCampos(false);
        editando = true;
        emit block();
        Configuracion_global->empresaDB.transaction();
        Configuracion_global->groupDB.transaction();

    }
    else
    {
        QMessageBox::warning(qApp->activeWindow(),tr("Editar Pedido"),tr("No se puede editar un pedido que ha sido facturado, solo los NO facturados se pueden editar")+
                             tr("<p><b> Si necesita modificar algo genere una factura nueva y realice el abono correspondiente</b>")+
                                tr("y luego si es preciso realice un nuevo pedido y facture de nuevo "),tr("OK"));
    }
}

void FrmPedidos::on_btnGuardar_clicked()
{
    LLenarPedido();
    bool succes = true;

    succes &= oPedido->GuardarPedido(oPedido->id);

    if(succes)
    {
       Configuracion_global->empresaDB.commit();
       Configuracion_global->groupDB.commit();

        TimedMessageBox * t = new TimedMessageBox(this,tr("Pedido guardado con éxito"));
        BloquearCampos(true);
        emit unblock();
    }
    else
    {
        QMessageBox::critical(this,tr("Error"),
                              tr("Error al guardar el pedido.\n")+Configuracion_global->empresaDB.lastError().text(),
                              tr("&Aceptar"));
        Configuracion_global->empresaDB.rollback();
        Configuracion_global->groupDB.rollback();
        if(Configuracion_global->contabilidad)
            Configuracion_global->contaDB.rollback();
    }
}

void FrmPedidos::on_botBuscarCliente_clicked()
{
    db_consulta_view consulta;
    QStringList campos;
    campos <<"nombre_fiscal" <<"codigo_cliente" << "cif_nif"<< "poblacion" << "telefono1";
    consulta.set_campoBusqueda(campos);
    consulta.set_texto_tabla("clientes");
    consulta.set_db("Maya");
    consulta.set_SQL("select id,codigo_cliente,nombre_fiscal,cif_nif,poblacion,telefono1 from clientes");
    QStringList cabecera;
    QVariantList tamanos;
    cabecera  << tr("Código") << tr("Nombre") << tr("CIF/NIF") << tr("Población") << tr("Teléfono");
    tamanos <<"0" << "100" << "300" << "100" << "180" <<"130";
    consulta.set_headers(cabecera);
    consulta.set_tamano_columnas(tamanos);
    consulta.set_titulo("Busqueda de Clientes");
    if(consulta.exec())
    {
        int id = consulta.get_id();
        oCliente3->Recuperar("select * from clientes where id="+QString::number(id));
        LLenarCamposCliente();
    }
}

void FrmPedidos::on_btnDeshacer_clicked()
{
    BloquearCampos(true);
    Configuracion_global->empresaDB.rollback();
    Configuracion_global->groupDB.rollback();
    QString cid = (ui->txtpedido->text());
    oPedido->RecuperarPedido("Select * from ped_cli where id ="+cid+" order by id limit 1 ");
    LLenarCampos();
    emit unblock();
}

void FrmPedidos::on_btn_borrar_clicked()
{
    if (QMessageBox::question(this,tr("Borrar"),
                              tr("Esta acción no se puede deshacer.\n¿Desea continuar?"),
                              tr("Cancelar"),
                               tr("Borrar"))== QMessageBox::Accepted)
    {
        bool succes = true;
        Configuracion_global->empresaDB.transaction();

        QSqlQuery q(Configuracion_global->empresaDB);
        succes = oPedido->BorrarLineas(oPedido->id);
        QString error;
        succes = SqlCalls::SqlDelete("ped_cli",Configuracion_global->empresaDB,
                                     QString("id=%1").arg(oPedido->id),error);

        if(succes)
            succes &= Configuracion_global->empresaDB.commit();

        if(succes)
        {
            TimedMessageBox * t = new TimedMessageBox(this,tr("Pedido borrado con éxito"));
            VaciarCampos();
            oPedido->id=-1;
            on_btnSiguiente_clicked();
        }
        else
            Configuracion_global->empresaDB.rollback();
    }
    filter_table(this->texto,this->orden,this->modo);
}

void FrmPedidos::lineaDeleted(lineaDetalle * ld)
{
    //todo borrar de la bd y stock y demas
    //if id = -1 pasando olimpicamente
    bool ok_empresa,ok_Maya;
    ok_empresa = true;
    ok_Maya = true;
    Configuracion_global->empresaDB.transaction();
    Configuracion_global->groupDB.transaction();
    if(ld->idLinea >-1)
    {
        //TODO control de stock
        QSqlQuery q(Configuracion_global->empresaDB);
        q.prepare("delete from lin_ped where id =:id");
        q.bindValue(":id",ld->idLinea);
        if(q.exec() && ok_Maya)
        {
            Configuracion_global->empresaDB.commit();
            Configuracion_global->groupDB.commit();
        } else
        {
            Configuracion_global->empresaDB.rollback();
            Configuracion_global->groupDB.rollback();
        }
    }
    delete ld;
}

void FrmPedidos::refrescar_modelo()
{
    modelLineas->setQuery(QString("select id,codigo,descripcion,cantidad,precio_recom, porc_dto, precio,subtotal,porc_iva,total "
                              "from lin_ped where id_cab = %1;").arg(oPedido->id),Configuracion_global->empresaDB);
}

void FrmPedidos::convertir_ealbaran()
{
    if(oPedido->albaran ==0)
    {
        QString c = QString("id = %1").arg(oPedido->id);
        QString error;
        QHash <QString, QVariant> h;
        h["pedido_cliente"] = oPedido->pedido;
        h["id_cliente"] = oPedido->id_cliente;
        h["codigo_cliente"] = oPedido->codigo_cliente;
        h["id_divisa"] =oPedido->id_divisa;
        h["cliente"] = oPedido->cliente;
        h["direccion1"] = oPedido->direccion1;
        h["direccion2"] = oPedido->direccion2;
        h["poblacion"] = oPedido->poblacion;
        h["provincia"] = oPedido->provincia;
        h["cp"] = oPedido->cp;
        h["id_pais"] = oPedido->id_pais;
        h["direccion1_entrega"] = oPedido->direccion_entrega1;
        h["direccion2_entrega"] = oPedido->direccion_entrega2;
        h["poblacion_entrega"] = oPedido->poblacion_entrega;
        h["provincia_entrega"] = oPedido->provincia_entrega;
        h["cp_entrega"] = oPedido->cp_entrega;
        h["id_pais_entrega"] = oPedido->id_pais_entrega;
        h["email_entrega"] = oPedido->email_entrega;
        h["comentarios_entrega"] = oPedido->comentarios_entrega;
        h["cif"] = oPedido->cif;
        h["telefono"] = oCliente3->telefono1;
        h["fax"] = oCliente3->movil;
        h["email"] = oCliente3->email;
        h["recargo_equivalencia"] = oPedido->recargo_equivalencia;
        h["subtotal"] = oPedido->subtotal;
        h["dto"] = oPedido->dto;
        h["porc_dto"] = oPedido->porc_dto;
        h["porc_dto_pp"] = oPedido->porc_dto_pp;
        h["dto_pp"] = oPedido->dto_pp;
        h["base1"] = oPedido->base1;
        h["base2"] = oPedido->base2;
        h["base3"] = oPedido->base3;
        h["base4"] = oPedido->base4;
        h["porc_iva1"] = oPedido->porc_iva1;
        h["porc_iva2"] = oPedido->porc_iva2;
        h["porc_iva3"] = oPedido->porc_iva3;
        h["porc_iva4"] = oPedido->porc_iva4;
        h["iva1"] = oPedido->iva1;
        h["iva2"] = oPedido->iva2;
        h["iva3"] = oPedido->iva3;
        h["iva4"] = oPedido->iva4;
        h["porc_rec1"] = oPedido->porc_rec1;
        h["porc_rec2"] = oPedido->porc_rec2;
        h["porc_rec3"] = oPedido->porc_rec3;
        h["porc_rec4"] = oPedido->porc_rec4;
        h["rec1"] = oPedido->rec1;
        h["rec2"] = oPedido->rec2;
        h["rec3"] = oPedido->rec3;
        h["rec4"] = oPedido->rec4;
        h["total1"] = oPedido->total1;
        h["total2"] = oPedido->total2;
        h["total3"] = oPedido->total3;
        h["total4"] = oPedido->total4;
        h["base_total"] = oPedido->base_total;
        h["iva_total"] = oPedido->iva_total;
        h["rec_total"] = oPedido->rec_total;
        h["total_albaran"] = oPedido->total_albaran;
        h["impreso"] = oPedido->impreso;
        h["facturado"] = oPedido->facturado;
        h["factura"] = oPedido->factura;
        h["fecha_factura"] = oPedido->fecha_factura;
        h["comentario"] = oPedido->comentario;
        h["entregado_a_cuenta"] = oPedido->entregado_a_cuenta;
        h["id_forma_pago"] = oPedido->id_forma_pago;
        h["desc_gasto1"] = oPedido->gasto1;
        h["desc_gasto2"] = oPedido->gasto2;
        h["desc_gasto3"] = oPedido->gasto3;
        h["imp_gasto1"] = oPedido->imp_gasto1;
        h["imp_gasto2"] = oPedido->imp_gasto2;
        h["imp_gasto3"] = oPedido->imp_gasto3;
        h["porc_iva_gasto1"] = oPedido->porc_iva_gasto1;
        h["porc_iva_gasto2"] = oPedido->porc_iva_gasto2;
        h["porc_iva_gasto3"] = oPedido->porc_iva_gasto3;
        h["iva_gasto1"] = oPedido->iva_gasto1;
        h["iva_gasto2"] = oPedido->iva_gasto2;
        h["iva_gasto3"] = oPedido->iva_gasto3;
        h["id_transportista"] = oPedido->id_transportista;
        h["ejercicio"] = Configuracion_global->cEjercicio;
        h["editable"] = true;


        Albaran oAlbaran;
        Configuracion_global->empresaDB.transaction();
        QString serie;
        if(Configuracion_global->serie.isEmpty())
        {
            //-----------------------
            // Elección de serie
            //-----------------------
            QDialog* dlg = new QDialog(this);
            dlg->setWindowTitle(tr("Seleccione serie albarán"));
            dlg->resize(170,150);
            QComboBox* box = new QComboBox(dlg);
            QPushButton*  btn = new QPushButton("Aceptar",dlg);
            QVBoxLayout lay(dlg);

            lay.addWidget(box);
            lay.addWidget(btn);

            dlg->setLayout(&lay);

            QSqlQueryModel *l = new QSqlQueryModel(this);
            l->setQuery("select serie from series",Configuracion_global->empresaDB);
            box->setModel(l);

            connect(btn,SIGNAL(clicked()),dlg,SLOT(accept()));
            dlg->exec();//aki se podria poner otro boton y cancelar todo?

             serie = box->currentText();
            dlg->deleteLater();
        } else
            serie = Configuracion_global->serie;

        int num = oAlbaran.NuevoNumeroAlbaran(serie);
        h["albaran"] =num;
        h["serie"] = serie;
        h["fecha"] = QDate::currentDate();

        int added = SqlCalls::SqlInsert(h,"cab_alb",Configuracion_global->empresaDB,error);
        // ----------------
        // Lineas de albaran
        // ----------------
        int added_l;
        QHash <QString, QVariant> h_l;
        QString d = QString("id_cab = %1").arg(oPedido->id);
        QMap<int, QSqlRecord> map = SqlCalls::SelectRecord("lin_ped", d,Configuracion_global->empresaDB, error);
          QMapIterator<int, QSqlRecord> i_l(map);
          while (i_l.hasNext())
          {
              i_l.next();
              QSqlRecord r_l = i_l.value();
              for(int i= 0; i< r_l.count();i++)
                  h_l.insert(r_l.fieldName(i),r_l.value(i));
              h_l.remove("id");
              h_l["id_cab"] = added;
              h_l.remove("cantidad_a_servir");
             added_l = SqlCalls::SqlInsert(h_l,"lin_alb",Configuracion_global->empresaDB,error);
             if(added_l == -1)
                 break;
          }
        if(added_l == -1 || added == -1)
        {
            Configuracion_global->empresaDB.rollback();
//            qDebug() <<added;
//            qDebug() <<error;
        } else
        {
            QHash <QString, QVariant> p;
            p["albaran"] = num;
            c = "id="+QString::number(oPedido->id);
            bool updated = SqlCalls::SqlUpdate(p,"ped_cli",Configuracion_global->empresaDB,c,error);

            if(updated)
            {

                t = new TimedMessageBox(qApp->activeWindow(),
                                                          QObject::tr("Se ha creado el albarán num:")+QString::number(num));
                oPedido->RecuperarPedido("select * from ped_cli where id ="+QString::number(oPedido->id));
                //-------------------------------------
                // Insertamos datos albaran en pedido
                //-------------------------------------
                oPedido->albaran =oAlbaran.albaran;
                oPedido->GuardarPedido(oPedido->id);
                ui->txtalbaran->setText(QString::number(num));
                bool commited =Configuracion_global->empresaDB.commit();

            } else
            {
                QMessageBox::warning(this,tr("Traspaso a albarán"),tr("No se pudo crear el albarán"),tr("Aceptar"));
                Configuracion_global->empresaDB.rollback();
            }
        }
    } else
        QMessageBox::information(this,tr("Traspasar presupuesto"),tr("Este presupuesto ya ha sido traspasado\nNo puede traspasar dos veces el mismo presupuesto"),
                                 tr("Aceptar"));
}

void FrmPedidos::convertir_enFactura()
{
    if(oPedido->editable)
    {
        if(QMessageBox::question(this,tr("Gestión de pedidos"),tr("¿Desea realmente facturar este pedido?"),
                                 tr("No"),tr("Sí"))==QMessageBox::Accepted)
        {
            LLenarPedido();
            if(ui->txtalbaran->text()=="0")
                convertir_ealbaran();

            QString serie;
            if(Configuracion_global->serie.isEmpty())
            {
                //--------------------
                // Preguntamos serie
                //--------------------
                QDialog* dlg = new QDialog(this);
                dlg->setWindowTitle(tr("Seleccione serie factura"));
                dlg->resize(270,150);
                QComboBox* box = new QComboBox(dlg);
                QPushButton*  btn = new QPushButton("Aceptar",dlg);
                QVBoxLayout lay(dlg);

                lay.addWidget(box);
                lay.addWidget(btn);

                dlg->setLayout(&lay);

                QSqlQueryModel *l = new QSqlQueryModel(this);
                l->setQuery("select serie from series",Configuracion_global->empresaDB);
                box->setModel(l);

                connect(btn,SIGNAL(clicked()),dlg,SLOT(accept()));
                dlg->exec();//aki se podria poner otro boton y cancelar todo?

                serie = box->currentText();
                dlg->deleteLater();
            } else
                serie = Configuracion_global->serie;

            Factura oFactura(this);
            Configuracion_global->empresaDB.transaction();
            Configuracion_global->groupDB.transaction();
            if(Configuracion_global->contabilidad)
                Configuracion_global->contaDB.transaction();
            bool transaccion = true;
            //-----------------------
            // LLeno objeto factura
            //-----------------------
            //oFactura.AnadirFactura();
            QHash <QString,QVariant> fact;
            oCliente3->Recuperar("select * from clientes where codigo_cliente ='"+ui->txtcodigo_cliente->text()+"'");
            fact["serie"] = serie;
            fact["cif"] = oPedido->cif;
            fact["cliente"] = oPedido->cliente;
            fact["id_divisa"] = oPedido->id_divisa;
            fact["codigo_cliente"] = oPedido->codigo_cliente;
            fact["id_cliente"] = oCliente3->id;
            fact["codigo_entidad"] = oCliente3->entidad_bancaria;
            fact["cuenta_corriente"] = oCliente3->cuenta_corriente;
            fact["oficina_entidad"] = oCliente3->oficina_bancaria;
            fact["dc_cuenta"] = oCliente3->dc;
            fact["cp"] = oPedido->cp;
            fact["direccion1"] = oPedido->direccion1;
            fact["direccion2"] = oPedido->direccion2;
            fact["id_forma_pago"] = oPedido->id_forma_pago;
            fact["pedido_cliente"] = ui->txtpedido->text().toInt();
            fact["poblacion"] =oPedido->poblacion;
            fact["provincia"] = oPedido->provincia;
            fact["fecha"] = QDate::currentDate();
            fact["id_pais"] = oPedido->id_pais;
            fact["direccion1"] = oPedido->direccion1;
            fact["direccion2"] = oPedido->direccion2;
            fact["poblacion"] =oPedido->poblacion;
            fact["provincia"] = oPedido->provincia;
            fact["id_pais"] = oPedido->id_pais;
            fact["direccion1_entrega"] = oPedido->direccion_entrega1;
            fact["direccion2_entrega"] = oPedido->direccion_entrega2;
            fact["poblacion_entrega"] =oPedido->poblacion_entrega;
            fact["provincia_entrega"] = oPedido->provincia_entrega;
            fact["id_pais_entrega"] = oPedido->id_pais_entrega;
            fact["id_forma_pago"] = Configuracion_global->Devolver_id_forma_pago(oCliente3->forma_pago);
            fact["recargo_equivalencia"] = oCliente3->recargo_equivalencia;
            fact["dto"] = oCliente3->porc_dto_cliente;
            fact["porc_iva1"] = oPedido->porc_iva1;
            fact["porc_iva2"] = oPedido->porc_iva2;
            fact["porc_iva3"] = oPedido->porc_iva3;
            fact["porc_iva4"] = oPedido->porc_iva4;
            fact["porc_rec1"] = oPedido->porc_rec1;
            fact["porc_rec2"] = oPedido->porc_rec2;
            fact["porc_rec3"] = oPedido->porc_rec3;
            fact["porc_rec4"] = oPedido->porc_rec4;
            fact["base"] = oPedido->base_total;
            fact["base1"] = oPedido->base1;
            fact["base2"] = oPedido->base2;
            fact["base3"] = oPedido->base3;
            fact["base4"] = oPedido->base4;
            fact["dto"] = oPedido->dto;
            fact["iva"] = oPedido->iva_total;
            fact["iva1"] = oPedido->iva1;
            fact["iva2"] = oPedido->iva2;
            fact["iva3"] = oPedido->iva3;
            fact["iva4"] = oPedido->iva4;
            fact["porc_rec1"] = oPedido->porc_rec1;
            fact["porc_rec2"] = oPedido->porc_rec2;
            fact["porc_rec3"] = oPedido->porc_rec3;
            fact["porc_rec4"] = oPedido->porc_rec4;
            fact["rec1"] = oPedido->rec1;
            fact["rec2"] = oPedido->rec2;
            fact["rec3"] = oPedido->rec3;
            fact["subtotal"] = oPedido->subtotal;
            fact["total"] = oPedido->total_pedido;
            fact["total1"] = oPedido->total1;
            fact["total2"] = oPedido->total2;
            fact["total3"] = oPedido->total3;
            fact["total4"] = oPedido->total4;
            fact["total_recargo"] = oPedido->rec_total;
            fact["ejercicio"] = Configuracion_global->cEjercicio.toInt();
            fact["desc_gasto1"] = oPedido->gasto1;
            fact["desc_gasto2"] = oPedido->gasto2;
            fact["desc_gasto3"] = oPedido->gasto3;
            fact["imp_gasto1"] = oPedido->imp_gasto1;
            fact["imp_gasto2"] = oPedido->imp_gasto2;
            fact["imp_gasto3"] = oPedido->imp_gasto3;
            fact["porc_iva_gasto1"] = oPedido->porc_iva_gasto1;
            fact["porc_iva_gasto2"] = oPedido->porc_iva_gasto2;
            fact["porc_iva_gasto3"] = oPedido->porc_iva_gasto3;
            fact["iva_gasto1"] = oPedido->iva_gasto1;
            fact["iva_gasto2"] = oPedido->iva_gasto2;
            fact["iva_gasto3"] = oPedido->iva_gasto3;


            // ----------------------------------
            // Creamos la cabecera de la factura
            //-----------------------------------
            fact["factura"] = oFactura.NuevoNumeroFactura(serie);
            QString error;
            int new_id_fact = SqlCalls::SqlInsert(fact,"cab_fac",Configuracion_global->empresaDB,error);
            if(new_id_fact >0)
            {
                //-------------------
                //  INSERTAR LÍNEAS
                //-------------------
                QSqlQuery lineas_ped(Configuracion_global->empresaDB);

                if(lineas_ped.exec("Select * from lin_ped where id_cab ="+QString::number(oPedido->id)))
                {
                    QHash <QString, QVariant> h_lineas_fac;
                    while (lineas_ped.next()) {

                        h_lineas_fac["id_cab"] = new_id_fact;
                        h_lineas_fac["id_articulo"] = lineas_ped.record().value("id_articulo").toInt();
                        h_lineas_fac["codigo"] = lineas_ped.record().value("codigo").toString();
                        h_lineas_fac["cantidad"] =  lineas_ped.record().value("cantidad").toInt();
                        h_lineas_fac["descripcion"] = lineas_ped.record().value("descripcion").toString();
                        h_lineas_fac["precio"] = lineas_ped.record().value("precio").toDouble();
                        h_lineas_fac["precio_recom"] = lineas_ped.record().value("precio_recom").toDouble();
                        h_lineas_fac["subtotal"] = lineas_ped.record().value("subtotal").toDouble();
                        h_lineas_fac["porc_dto"] = lineas_ped.record().value("porc_dto").toDouble();
                        h_lineas_fac["dto"] = lineas_ped.record().value("dto").toDouble();
                        h_lineas_fac["porc_iva"] = lineas_ped.record().value("porc_iva").toDouble();
                        h_lineas_fac["total"] = lineas_ped.record().value("total").toDouble();
                        int new_id = SqlCalls::SqlInsert(h_lineas_fac,"lin_fac",Configuracion_global->empresaDB,error);

                        if(new_id == -1)
                        {
                            QMessageBox::warning(this,tr("Pedidos cliente"),
                                                 tr("Ocurrió un error al crear las líneas de factura: %1").arg(error));

                            transaccion = false;
                        }
                    }
                }




                if(transaccion) {
                    ui->btn_convertir->setEnabled(false);
                    QString texto;
                    texto = tr("Se ha creado una nueva factura.\ncon el número ")+ fact.value("factura").toString()+
                               tr("\n y de importe: ")+Configuracion_global->toFormatoMoneda(
                                QString::number(fact.value("total").toDouble(),'f',Configuracion_global->decimales_campos_totales));
                    TimedMessageBox * t = new TimedMessageBox(this,texto);
                    //-------------------------------------
                    // Insertamos datos factura en pedido
                    //-------------------------------------

                    QHash <QString, QVariant> ped;
                    ped["factura"] = fact.value("factura").toString();
                    ped["facturado"] = true;
                    ui->lblfacturado->setVisible(true);
                    ui->txtcNumFra->setText(fact.value("factura").toString());
                    ped["editable"] = false;
                    ped["traspasado_albaran"]= true;
                    ped["traspasado_factura"] = true;
                    bool success = SqlCalls::SqlUpdate(ped,"ped_cli",Configuracion_global->empresaDB,QString("id=%1").arg(oPedido->id),error);
                    if (success)
                    {

                        Configuracion_global->empresaDB.commit();
                        Configuracion_global->groupDB.commit();
                        if(Configuracion_global->contabilidad)
                            Configuracion_global->contaDB.commit();

                        //------------------------------------
                        // Actualizamos factura en albarán
                        //------------------------------------
                        QString error;
                        int albaran = SqlCalls::SelectOneField("ped_cli","albaran",QString("albaran=%1").arg(oPedido->albaran),
                                                                Configuracion_global->empresaDB,error).toInt();
                        if(albaran >0)
                        {
                            QHash <QString, QVariant> a;
                            a["factura"] = oFactura.factura;
                            bool updated = SqlCalls::SqlUpdate(a,"cab_alb",Configuracion_global->empresaDB,
                                                               QString("albaran =%1").arg(oPedido->albaran),
                                                               error);
                            if(!updated)
                                TimedMessageBox *t = new TimedMessageBox(this,tr("No se pudo guardar el numero de albarán en la factura: %1").arg(error));
                        }



                        vencimientos vto(this);
                        //-----------------
                        // Asiento contable
                        //-----------------
                        if(Configuracion_global->contabilidad)
                        {

                            bool creado;
                            oCliente3->Recuperar("select * from clientes where id = "+QString::number(oFactura.id_cliente));
                            if(oFactura.apunte == 0)
                                creado = oFactura.Apunte();
                            else
                                creado = oFactura.EditApunte(oFactura.apunte);
                            if(!creado){
                                Configuracion_global->groupDB.rollback();
                                Configuracion_global->empresaDB.rollback();
                                Configuracion_global->empresaDB.rollback();
                                QMessageBox::warning(this,tr("Gestión de Pedidos"),
                                                     tr("Ocurrió un error al traspasar el pedido y crear el asiento contable\n se desharan los cambios y se anulará la factura"),
                                                     tr("Aceptar"));

                            }
                        }
                        //------------------
                        // Vencimiento
                        //------------------
                        vto.calcular_vencimiento(oFactura.fecha,oFactura.id_cliente,0,oFactura.id,(oFactura.serie+"/"+oFactura.factura),1,
                                                 "V",oFactura.total);
                        oPedido->RecuperarPedido("select * from ped_cli where id ="+QString::number(oPedido->id));
                        LLenarCampos();

                } else
                {

                    QMessageBox::warning(this,tr("Traspaso a factura"),tr("No se pudo crear la factura "),tr("Aceptar"));

                    Configuracion_global->empresaDB.rollback();
                    Configuracion_global->groupDB.rollback();
                    if(Configuracion_global->contabilidad)
                        Configuracion_global->contaDB.rollback();
                }





                } else
                {
                    Configuracion_global->empresaDB.rollback();
                    QMessageBox::warning(this,tr("Gestión de pedidos"),
                                         tr("No se ha podido crear la factura"),tr("Aceptar"));
                }
            } else
            {

                QMessageBox::warning(this,tr("Traspaso a factura"),tr("No se pudo crear la factura "),tr("Aceptar"));

                Configuracion_global->empresaDB.rollback();
                Configuracion_global->groupDB.rollback();
                if(Configuracion_global->contabilidad)
                    Configuracion_global->contaDB.rollback();
            }
        }
    } else
        QMessageBox::warning(this,tr("Gestión de pedidos"),tr("Este pedido ya ha sido traspasado"),tr("Aceptar"));

}

void FrmPedidos::on_tabWidget_2_currentChanged(int index)
{
    Q_UNUSED(index);
    //helper.resizeTable();
}


void FrmPedidos::on_txtcodigo_cliente_editingFinished()
{
    if(ui->txtcodigo_cliente->text() != oPedido->codigo_cliente)
    {
        oCliente3->decrementar_acumulados(oCliente3->id,oPedido->total_pedido,QDate::currentDate());
    }
    if((ui->txtcodigo_cliente->text() != oPedido->codigo_cliente)&& !ui->txtcodigo_cliente->text().isEmpty()){
        if(ui->txtcodigo_cliente->text().trimmed().size() < Configuracion_global->digitos_cuentas_contables)
        {
            QString ccod= Configuracion_global->cuenta_clientes;
            QString ccli = ui->txtcodigo_cliente->text().trimmed();
            int tam = Configuracion_global->digitos_cuentas_contables - (ccod.size() + ccli.size());
            QString zeros;
            for(int i = 0; i<tam;i++)
            {
                zeros.append("0");
            }
            ui->txtcodigo_cliente->setText(ccod+zeros+ccli);

        }
        if(!ui->txtcodigo_cliente->text().isEmpty())
        {
            oCliente3->Recuperar("select * from clientes where codigo_cliente='"+ui->txtcodigo_cliente->text()+"'");
            oPedido->id_cliente = oCliente3->id;
            LLenarCamposCliente();
        }
    }
}

void FrmPedidos::on_btnImprimir_clicked()
{
    FrmDialogoImprimir dlg_print(this);
    dlg_print.set_email(oCliente3->email);
    dlg_print.set_preview(false);
    if(dlg_print.exec() == dlg_print.Accepted)
    {
        ui->lblimpreso->setVisible(true);
        oPedido->impreso == true;
        if(ui->btnEditar->isEnabled())
        {
            QHash <QString, QVariant> h;
            h["impreso"] = true;
            QString error;
            SqlCalls::SqlUpdate(h,"ped_cli",Configuracion_global->empresaDB,QString("id=%1").arg(oPedido->id),error);

        }
        int valor = dlg_print.get_option();
        QMap <QString,QString> parametros_sql;
        parametros_sql["Empresa.ped_cli"] = QString("id = %1").arg(oPedido->id);
        parametros_sql["Empresa.lin_ped"] = QString("id_cab = %1").arg(oPedido->id);
        QString report = "pedido_"+QString::number(oCliente3->ididioma);


        QMap <QString,QString> parametros;
        //TODO parametros
        switch (valor) {
        case 1: // Impresora
            Configuracion::ImprimirDirecto(report,parametros_sql,parametros);
            break;
        case 2: // email
            // TODO - enviar pdf por mail
            break;
        case 3: // PDF
            Configuracion::ImprimirPDF(report,parametros_sql,parametros);
            break;
        case 4: //preview
            Configuracion::ImprimirPreview(report,parametros_sql,parametros);
            break;
        default:
            break;
        }
    }
}

void FrmPedidos::on_radBusqueda_toggled(bool checked)
{
    if(checked)
        ui->stackedWidget->setCurrentIndex(1);
    else
        ui->stackedWidget->setCurrentIndex(0);

}

void FrmPedidos::on_tabla_clicked(const QModelIndex &index)
{
    int id = ui->tabla->model()->data(ui->tabla->model()->index(index.row(),0),Qt::EditRole).toInt();
    oPedido->id = id;
//    oPedido->RecuperarPedido("select * from ped_cli where id ="+QString::number(id));
//    //LLenarCampos();
 //   BloquearCampos(true);
}

void FrmPedidos::on_tabla_doubleClicked(const QModelIndex &index)
{
    int id = ui->tabla->model()->data(ui->tabla->model()->index(index.row(),0),Qt::EditRole).toInt();
    oPedido->RecuperarPedido("select * from ped_cli where id ="+QString::number(id));
    LLenarCampos();
    QString filter = QString("id_Cab = '%1'").arg(oPedido->id);
    //helper.fillTable("empresa","lin_ped",filter);
    ui->stackedWidget->setCurrentIndex(0);
    ocultarBusqueda();
    BloquearCampos(true);
    refrescar_modelo();
}


void FrmPedidos::on_btnImprimir_2_clicked()
{
    on_btnImprimir_clicked();
}


void FrmPedidos::on_spin_porc_dto_pp_editingFinished()
{
    float dto_pp;
    dto_pp = (Configuracion_global->MonedatoDouble(ui->txtsubtotal->text())*(ui->spinPorc_dto_pp->value()/100.0));
    ui->txtDto_pp->setText(Configuracion_global->toFormatoMoneda(QString::number(dto_pp,'f',Configuracion_global->decimales_campos_totales)));
    oPedido->dto_pp = dto_pp;
    //helper.fillTable("empresa","lin_ped",QString("id_cab =%1").arg(oPedido->id));
}

void FrmPedidos::on_spin_porc_dto_especial_editingFinished()
{
    calcular_pedido();
}


void FrmPedidos::cboporc_iva_gasto1_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    calcular_iva_gastos();

}

void FrmPedidos::cboporc_iva_gasto2_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    calcular_iva_gastos();

}

void FrmPedidos::cboporc_iva_gasto3_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    calcular_iva_gastos();
}

void FrmPedidos::SpinGastoDist1_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    calcular_iva_gastos();

}
void FrmPedidos::SpinGastoDist2_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    calcular_iva_gastos();

}
void FrmPedidos::SpinGastoDist3_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    calcular_iva_gastos();
;
}

void FrmPedidos::mostrarBusqueda()
{
    ui->stackedWidget->setCurrentIndex(1);
    _showBarraBusqueda(m_busqueda);
    m_busqueda->doFocustoText();
}

void FrmPedidos::ocultarBusqueda()
{
    _hideBarraBusqueda(m_busqueda);
}

void FrmPedidos::on_Lineas_doubleClicked(const QModelIndex &index)
{
    if(ui->btnGuardar->isEnabled())
    {
        int id_lin = ui->Lineas->model()->data(index.model()->index(index.row(),0)).toInt();
        if(id_lin >0)
        {

            frmEditLine frmeditar(this);
            connect(&frmeditar,SIGNAL(refrescar_lineas()),this,SLOT(refrescar_modelo()));
            frmeditar.set_acumula(false);
            frmeditar.set_id_cliente(oCliente3->id);
            frmeditar.set_id_tarifa(oCliente3->idTarifa);
            frmeditar.set_id_cab(oPedido->id);
            frmeditar.set_tipo("V");
            frmeditar.set_linea(id_lin,"lin_ped");
            frmeditar.set_tabla("lin_ped");
            frmeditar.set_editando();
            frmeditar.exec();

            calcular_pedido();
            ui->Lineas->setFocus();

        } else
        {
            QMessageBox::warning(this, tr("Gestión de Pedidos"),tr("Debe editar el pedido para poder modificar las líneas"),
                                 tr("Aceptar"));

        }
    }
}

void FrmPedidos::on_btnAnadirLinea_clicked()
{
    if(ui->btnGuardar->isEnabled())
    {
            frmEditLine frmeditar(this);
            connect(&frmeditar,SIGNAL(refrescar_lineas()),this,SLOT(refrescar_modelo()));
            frmeditar.set_acumula(false);
            frmeditar.set_linea(0,"lin_ped");
            frmeditar.set_tabla("lin_ped");
            frmeditar.set_id_cliente(oCliente3->id);
            frmeditar.set_id_tarifa(oCliente3->idTarifa);
            frmeditar.set_id_cab(oPedido->id);
            frmeditar.set_tipo("V");
            if(!frmeditar.exec() == QDialog::Accepted)
                refrescar_modelo();
                calcular_pedido();

    } else{
        QMessageBox::warning(this,tr("Gestión de Pedidos"),tr("Debe editar el pedido para añadir líneas"),
                             tr("Aceptar"));
    }
}

void FrmPedidos::on_btn_borrarLinea_clicked()
{
    if(QMessageBox::question(this,tr("Lineas de pedidos"), tr("¿Borrar la linea?"),
                             tr("No"),tr("Borrar")) == QMessageBox::Accepted)
    {
        QModelIndex index = ui->Lineas->currentIndex();
        int id_lin = ui->Lineas->model()->index(index.row(),0).data().toInt();
        oPedido->BorrarLineas(id_lin);
        modelLineas->setQuery(QString("select id,codigo,descripcion,cantidad,precio,precio_recom,subtotal,porc_dto,porc_iva,total "
                                      "from lin_ped where id_cab = %1;").arg(oPedido->id),Configuracion_global->empresaDB);
        calcular_pedido();
        ui->Lineas->setFocus();

    }
}
