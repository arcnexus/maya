#ifndef FRMPERSONASCONTACTOCLIENTE_H
#define FRMPERSONASCONTACTOCLIENTE_H

#include <QDialog>
#include <Zona_Pacientes/cliente.h>

namespace Ui {
class frmPersonasContactoCliente;
}

class frmPersonasContactoCliente : public QDialog
{
    Q_OBJECT
    
public:
    explicit frmPersonasContactoCliente(QWidget *parent = 0);
    ~frmPersonasContactoCliente();
    int nIdCliente;
   void  RefrescarTabla();
    Cliente oCliente;

private:
    Ui::frmPersonasContactoCliente *ui;



private slots:
    void Anadir();
    void Borrar();

};

#endif // FRMPERSONASCONTACTOCLIENTE_H
