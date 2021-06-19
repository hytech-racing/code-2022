#ifndef MC_FAULT_INFO_H
#define MC_FAULT_INFO_H

#include <QDialog>

namespace Ui {
class mc_fault_info;
}

class mc_fault_info : public QDialog
{
    Q_OBJECT

public:
    explicit mc_fault_info(QWidget *parent = 0);
    void set_post_fault(int post);
    void set_run_fault(int run);
    ~mc_fault_info();

private:
    Ui::mc_fault_info *ui;
};

#endif // MC_FAULT_INFO_H
