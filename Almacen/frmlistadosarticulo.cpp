#include "frmlistadosarticulo.h"
#include "ui_frmlistadosarticulo.h"



frmListadosArticulo::frmListadosArticulo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmListadosArticulo)
{
    ui->setupUi(this);
    connect(ui->btnPrevison,SIGNAL(clicked()),this,SLOT(btn_Preview_clicked()));
}

frmListadosArticulo::~frmListadosArticulo()
{
    delete ui;
}

void frmListadosArticulo::btn_Preview_clicked()
{
    //Configuracion_global->imprimir(false,true,this);
}
