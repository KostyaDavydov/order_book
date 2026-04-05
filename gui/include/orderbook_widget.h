#pragma once

#include "orderbook.h"
#include "qcustomplot.h"

#include <QTableWidget>
#include <QLabel>
#include <QMainWindow>
#include <QWidget>
#include <QThread>

//=================================================
//=================================================

class OrderBookWidget : public QMainWindow
{
    Q_OBJECT

private:
// Widgets
    QTableWidget * mp_asksTblWgt;
    QTableWidget * mp_bidsTblWgt;
    QLabel * mp_marketPriceLbl;
    QLabel * mp_spreadLbl;

    QCustomPlot * mp_DOMCustomPlot;

// Other fields
    bool m_pauseSimulation;
    double m_marketPrice;

public:
    explicit OrderBookWidget(QWidget * parent = nullptr);
    ~OrderBookWidget();
};
