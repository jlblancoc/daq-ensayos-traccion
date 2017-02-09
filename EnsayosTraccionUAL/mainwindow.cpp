/*+-------------------------------------------------------------------------+
|                             LibreDAQ                                    |
|                                                                         |
| Copyright (C) 2015  Jose Luis Blanco Claraco                            |
| Distributed under GNU General Public License version 3                  |
|   See <http://www.gnu.org/licenses/>                                    |
+-------------------------------------------------------------------------+  */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets/qmessagebox.h>
#include <QFileDialog>
#include <mrpt/math/CMatrixD.h>


QVector<double> plot_t, plot_x, plot_y; // initialize with entries 0..100
unsigned int uc_first_timestamp = 0;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->plot->addGraph();

	for (int i = 0; i < 3; i++) {
		ui->cbSerial->addItem(QString::asprintf("ttyUSB%i",i));
	}
	for (int i = 0; i < 10; i++) {
		ui->cbSerial->addItem( QString::asprintf("COM%i",i));
	}

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timer_update()));
	timer->start(100);

	m_arduino_daq.set_ADC_readings_callback(std::bind(&MainWindow::on_adc_data, this, std::placeholders::_1));
}

MainWindow::~MainWindow()
{
	delete ui;
}

// Rescan devices:
void MainWindow::on_actionRe_scan_devices_triggered()
{
	try
	{
		m_arduino_daq.setSerialPort(ui->cbSerial->currentText().toStdString());

		if (!m_arduino_daq.initialize())
			throw std::runtime_error("No se pudo conectar con la tarjeta DAQ!");

		m_arduino_daq.CMD_ADC_STOP();
	}
	catch (std::exception &e)
	{
		QMessageBox msg(QMessageBox::Critical, "Error", e.what(), QMessageBox::Ok, this);
		msg.exec();
	}
}

void MainWindow::on_btnCapturar_clicked()
{
}

void MainWindow::on_actionCapture_triggered()
{
	try
	{
		plot_t.clear();
		plot_x.clear();
		plot_y.clear();
		uc_first_timestamp = 0;

		TFrameCMD_ADC_start_payload_t adc_cfg;
		adc_cfg.active_channels[0] = 0;
		adc_cfg.active_channels[1] = 1;
		adc_cfg.measure_period_ms = 50;
		adc_cfg.use_internal_refvolt = true;

		m_arduino_daq.CMD_ADC_START(adc_cfg);

	}
	catch (std::exception &e)
	{
		QMessageBox msg(QMessageBox::Critical, "Error", e.what(), QMessageBox::Ok, this);
		msg.exec();
	}
	
}

void MainWindow::on_actionStop_triggered()
{
	try
	{
		m_arduino_daq.CMD_ADC_STOP();
	}
	catch (std::exception &e)
	{
		QMessageBox msg(QMessageBox::Critical, "Error", e.what(), QMessageBox::Ok, this);
		msg.exec();
	}
}

void MainWindow::on_btnGrabar_clicked()
{
	try {
		const size_t N = plot_t.size();
		if (!N)
			throw std::runtime_error("No hay datos capturados!");

		auto fileName = QFileDialog::getSaveFileName(this,"Grabar log", ".", "Text files (*.txt)");
		if (fileName.isEmpty())
			return;

		mrpt::math::CMatrixD M(N, 3);
		for (unsigned int i = 0; i < N; i++)
		{
			M(i, 0) = plot_t[i];
			M(i, 1) = plot_x[i];
			M(i, 2) = plot_y[i];
		}

		M.saveToTextFile(fileName.toStdString());
	}
	catch (std::exception &e)
	{
		QMessageBox msg(QMessageBox::Critical, "Error", e.what(), QMessageBox::Ok, this);
		msg.exec();
	}
}

void MainWindow::on_btnCargar_clicked()
{

}

void MainWindow::timer_update()
{
	try
	{
		m_arduino_daq.iterate();


	}
	catch (std::exception &e)
	{
		QMessageBox msg(QMessageBox::Critical, "Error", e.what(), QMessageBox::Ok, this);
		msg.exec();
	}
}

void MainWindow::on_adc_data(TFrame_ADC_readings_payload_t data)
{
	if (!uc_first_timestamp) {
		uc_first_timestamp = data.timestamp_ms;
	}

	const double t = (data.timestamp_ms - uc_first_timestamp)*1e-3;
	const double v1 = data.adc_data[0] * (1.1 / 1023) /* ADC conv */  * 11 /* resistor divisor */;
	const double v2 = data.adc_data[1] * (1.1 / 1023) /* ADC conv */ * 11 /* resistor divisor */;

	plot_t.push_back(t);
	plot_x.push_back(v1);
	plot_y.push_back(v2);

	ui->statusBar->showMessage(QString::asprintf("t=%.03f sec V1=%.03f V V2=%.03f V", t, v1, v2));

	refresh_plot();
}

void MainWindow::refresh_plot()
{
	const double scale_x = ui->comboBox->currentText().toDouble();
	const double scale_y = ui->comboBox_2->currentText().toDouble();

	QVector<double> xs(plot_x.size()), ys(plot_x.size());
	double max_x = 1.0, max_y = 1.0;
	for (size_t i = 0; i < plot_x.size(); i++)
	{
		xs[i] = plot_x[i] * scale_x;
		ys[i] = plot_y[i] * scale_y;

		mrpt::utils::keep_max(max_x, xs[i]);
		mrpt::utils::keep_max(max_y, ys[i]);
	}

	ui->plot->graph(0)->setData(xs, ys);
	// give the axes some labels:
	ui->plot->xAxis->setLabel("Alargamiento (cm)");
	ui->plot->yAxis->setLabel("Fuerza (kN)");

	// set axes ranges, so we see all data:
	ui->plot->xAxis->setRange(0, max_x*1.2);
	ui->plot->yAxis->setRange(0, max_y*1.2);
	ui->plot->replot();
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
	refresh_plot();
}

void MainWindow::on_comboBox_2_currentIndexChanged(const QString &arg1)
{
	refresh_plot();
}
