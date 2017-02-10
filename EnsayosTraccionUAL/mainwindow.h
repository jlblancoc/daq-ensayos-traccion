/*+-------------------------------------------------------------------------+
|                             LibreDAQ                                    |
|                                                                         |
| Copyright (C) 2015  Jose Luis Blanco Claraco                            |
| Distributed under GNU General Public License version 3                  |
|   See <http://www.gnu.org/licenses/>                                    |
+-------------------------------------------------------------------------+  */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <arduino_daq/ArduinoDAQ_LowLevel.h>

#include <deque>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_actionRe_scan_devices_triggered();
	void on_actionCapture_triggered();
	void on_actionStop_triggered();
	void on_btnCapturar_clicked();

    void on_btnGrabar_clicked();

    void on_btnCargar_clicked();

	void timer_update();

    void on_comboBox_currentIndexChanged(int index);

    void on_comboBox_2_currentIndexChanged(const QString &arg1);

private:
	Ui::MainWindow *ui;

	ArduinoDAQ_LowLevel m_arduino_daq;

	void on_adc_data(TFrame_ADC_readings_payload_t data);

	void refresh_plot();

};

#endif // MAINWINDOW_H
