from PySide2 import QtCore as Qc
from PySide2 import QtWidgets as Qw
import asyncio

from jumpSlider import JumpSlider


class Equalizer(Qw.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.freq_bands = [31, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000]
        self.band_widgets = []

        self.set_enable_btn = Qw.QPushButton("Enable Equalizer")
        self.set_enable_btn.setCheckable(True)
        self.set_enable_btn.toggled.connect(self.set_equalizer_enable)

        self.set_defaults_btn = Qw.QPushButton("Reset")
        self.set_defaults_btn.clicked.connect(self.set_defaults)

        self.toolbar_layout = Qw.QHBoxLayout()
        self.toolbar_layout.addWidget(self.set_enable_btn)
        self.toolbar_layout.addWidget(self.set_defaults_btn)

        self.bands_layout = Qw.QHBoxLayout()

        for idx, b_freq in enumerate(self.freq_bands):
            if b_freq > 999:
                band_w = EqualizerBand(f'{int(b_freq / 1000) if b_freq % 1000 == 0 else (b_freq / 1000)} kHz', parent=self)
            else:
                band_w = EqualizerBand(f'{b_freq} Hz', parent=self)

            band_w.gain_box.valueChanged.connect(lambda x, idx=idx: asyncio.ensure_future(self.do_band_update(idx, dB=x)))

            self.band_widgets.append(band_w)
            self.bands_layout.addWidget(band_w)

        self.layout = Qw.QVBoxLayout()
        self.layout.setContentsMargins(0, 0, 0, 0)
        self.layout.addLayout(self.toolbar_layout)
        self.layout.addLayout(self.bands_layout)

        self.setLayout(self.layout)

        self.show()


    async def load_settings(self):
        await self.parent().parent().bt_man.read_eq_gains()
        gains = self.parent().parent().bt_man.eq_gains

        for idx, band_w in enumerate(self.band_widgets):
            band_w.set_gain_no_signal(gains[idx])

        await self.parent().parent().bt_man.read_eq_enable()
        set_enable = self.parent().parent().bt_man.eq_enable
        self.set_enable_btn.blockSignals(True)
        if set_enable:
             self.set_enable_btn.setText("Disable Equalizer")
        if not set_enable:
             self.set_enable_btn.setText("Enable Equalizer")
        self.set_enable_btn.setChecked(set_enable)
        self.set_enable_btn.blockSignals(False)


    def set_equalizer_enable(self):
        if self.set_enable_btn.isChecked():
            asyncio.ensure_future(self.parent().parent().bt_man.write_eq_enable(True))
            self.set_enable_btn.setText("Disable Equalizer")
        else:
            asyncio.ensure_future(self.parent().parent().bt_man.write_eq_enable(False))
            self.set_enable_btn.setText("Enable Equalizer")


    def set_defaults(self):
        for band_w in self.band_widgets:
            band_w.set_gain_no_signal(0)

        asyncio.ensure_future(self.parent().parent().bt_man.write_eq_gains([0 for i in range(0, 10)]))


    async def do_band_update(self, band_num, dB):
        await self.parent().parent().bt_man.write_eq_gain_index(band_num, dB)
        self.parent().parent().update_conn_status()


class EqualizerBand(Qw.QWidget):
    def __init__(self, center_freq: str, parent=None):
        super().__init__(parent)
        self.MAX_GAIN = 10
        self.MIN_GAIN = -20

        self.center_freq = center_freq

        # Text box for entering and displaying gain values
        self.gain_box = Qw.QDoubleSpinBox()
        self.gain_box.setMinimum(self.MIN_GAIN)
        self.gain_box.setMaximum(self.MAX_GAIN)
        self.gain_box.setSingleStep(0.1)
        self.gain_box.setDecimals(1)
        self.gain_box.setButtonSymbols(Qw.QAbstractSpinBox.NoButtons)
        self.gain_box.setFixedWidth(40)
        self.gain_box.setAlignment(Qc.Qt.AlignVCenter)
        self.gain_box.setSizePolicy(
            Qw.QSizePolicy.Fixed, Qw.QSizePolicy.Fixed)

        # Label showing unit (decibels)
        self.unit_label = Qw.QLabel("dB")
        self.unit_label.setFixedWidth(15)
        self.unit_label.setAlignment(Qc.Qt.AlignVCenter)
        self.unit_label.setSizePolicy(
            Qw.QSizePolicy.Fixed, Qw.QSizePolicy.Fixed)

        # Gain slider
        self.slider = JumpSlider(Qc.Qt.Vertical, parent=self)
        self.slider.setMinimum(self.MIN_GAIN * 10)
        self.slider.setMaximum(self.MAX_GAIN * 10)
        self.slider.setSingleStep(1)
        self.slider.setPageStep(1)
        self.slider.setTickInterval(10)
        self.slider.setTickPosition(Qw.QSlider.TicksBothSides)
        self.slider.setSizePolicy(
            Qw.QSizePolicy.Fixed, Qw.QSizePolicy.Expanding)

        # Link text box and slider
        self.gain_box.valueChanged.connect(lambda x: self.slider.setValue(x * 10))
        self.slider.valueChanged.connect(lambda x: self.gain_box.setValue(x / 10))

        # Frequency label
        self.freq_label = Qw.QLabel(self.center_freq, parent=self)
        self.freq_label.setAlignment(Qc.Qt.AlignVCenter)
        self.freq_label.setSizePolicy(
            Qw.QSizePolicy.Fixed, Qw.QSizePolicy.Fixed)

        # Align gain text box and unit label horizontally
        self.inner_layout = Qw.QHBoxLayout()
        self.inner_layout.setContentsMargins(0, 0, 0, 0)
        self.inner_layout.addWidget(self.gain_box)
        self.inner_layout.addWidget(self.unit_label)
        self.inner_layout.setAlignment(Qc.Qt.AlignVCenter)

        self.outer_layout = Qw.QVBoxLayout(self)
        self.outer_layout.setContentsMargins(0, 0, 0, 0)
        self.outer_layout.addLayout(self.inner_layout)
        self.outer_layout.addWidget(self.slider)
        self.outer_layout.addWidget(self.freq_label)
        self.outer_layout.setAlignment(Qc.Qt.AlignVCenter)
        self.setLayout(self.outer_layout)
        self.show()

    def set_gain_no_signal(self, gain: int):
        self.gain_box.blockSignals(True)
        self.gain_box.setValue(gain)
        self.slider.setValue(gain * 10)
        self.gain_box.blockSignals(False)
