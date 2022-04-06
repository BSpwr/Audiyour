from PySide2 import QtCore as Qc
from PySide2 import QtWidgets as Qw
import asyncio

from jumpSlider import JumpSlider


class Mixer(Qw.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.tracks = ['3.5mm Jack', 'Bluetooth']
        self.band_widgets = []

        self.string_enable_jack_input = "Enable 3.5mm Jack Input"
        self.string_disable_jack_input = "Disable 3.5mm Jack Input"
        self.string_enable_bluetooth_input = "Enable Bluetooth Input"
        self.string_enable_bluetooth_input = "Disable Bluetooth Input"
        self.string_reset = "Reset"

        self.jack_enable_btn = Qw.QPushButton(self.string_enable_jack_input)
        self.jack_enable_btn.setCheckable(True)
        self.jack_enable_btn.toggled.connect(self.jack_mixer_enable)

        self.wireless_enable_btn = Qw.QPushButton(self.string_enable_bluetooth_input)
        self.wireless_enable_btn.setCheckable(True)
        self.wireless_enable_btn.toggled.connect(self.wireless_mixer_enable)

        self.set_defaults_btn = Qw.QPushButton(self.string_reset)
        self.set_defaults_btn.clicked.connect(self.set_defaults)

        self.toolbar_layout = Qw.QHBoxLayout()
        self.toolbar_layout.setContentsMargins(0, 0, 0, 0)
        self.toolbar_layout.addWidget(self.jack_enable_btn)
        self.toolbar_layout.addWidget(self.wireless_enable_btn)
        self.toolbar_layout.addWidget(self.set_defaults_btn)

        self.bands_layout = Qw.QHBoxLayout()
        self.bands_layout.setContentsMargins(0, 0, 0, 0)

        for idx, b_freq in enumerate(self.tracks):
            band_w = MixerBand(b_freq, parent=self)

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
        await self.parent().parent().bt_man.read_mix_gains()
        gains = self.parent().parent().bt_man.mix_gains

        for idx, band_w in enumerate(self.band_widgets):
            band_w.set_gain_no_signal(gains[idx])

        await self.parent().parent().bt_man.read_mix_line_in_en()
        jack_enable = self.parent().parent().bt_man.mix_line_in_en
        self.jack_enable_btn.blockSignals(True)
        self.jack_enable_btn.setChecked(jack_enable)
        if self.jack_enable_btn.isChecked():
            self.jack_enable_btn.setText(self.string_disable_jack_input)
        else:
            self.jack_enable_btn.setText(self.string_enable_jack_input)
        self.jack_enable_btn.blockSignals(False)

        await self.parent().parent().bt_man.read_mix_wireless_in_en()
        wireless_enable = self.parent().parent().bt_man.mix_wireless_in_en
        self.wireless_enable_btn.blockSignals(True)
        self.wireless_enable_btn.setChecked(wireless_enable)
        if self.wireless_enable_btn.isChecked():
            self.wireless_enable_btn.setText(self.string_enable_bluetooth_input)
        else:
            self.wireless_enable_btn.setText(self.string_enable_bluetooth_input)
        self.wireless_enable_btn.blockSignals(False)


    def jack_mixer_enable(self):
        if self.jack_enable_btn.isChecked():
            asyncio.ensure_future(self.parent().parent().bt_man.write_mix_line_in_en(True))
            self.jack_enable_btn.setText(self.string_disable_jack_input)
        else:
            asyncio.ensure_future(self.parent().parent().bt_man.write_mix_line_in_en(False))
            self.jack_enable_btn.setText(self.string_enable_jack_input)


    def wireless_mixer_enable(self):
        if self.wireless_enable_btn.isChecked():
            asyncio.ensure_future(self.parent().parent().bt_man.write_mix_wireless_in_en(True))
            self.wireless_enable_btn.setText(self.string_enable_bluetooth_input)
        else:
            asyncio.ensure_future(self.parent().parent().bt_man.write_mix_wireless_in_en(False))
            self.wireless_enable_btn.setText(self.string_enable_bluetooth_input)


    def set_defaults(self):
        for band_w in self.band_widgets:
            band_w.set_gain_no_signal(0)

        asyncio.ensure_future(self.parent().parent().bt_man.write_mix_gains([0 for i in range(0, 2)]))


    async def do_band_update(self, band_num, dB):
        await self.parent().parent().bt_man.write_mix_gain_index(band_num, int(dB))
        self.parent().parent().update_conn_status()


class MixerBand(Qw.QWidget):
    def __init__(self, label: str, parent=None):
        super().__init__(parent)
        self.MAX_GAIN = 20
        self.MIN_GAIN = -40

        self.label = label

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
        self.slider = JumpSlider(Qc.Qt.Horizontal, parent=self)
        self.slider.setMinimum(self.MIN_GAIN * 10)
        self.slider.setMaximum(self.MAX_GAIN * 10)
        self.slider.setSingleStep(1)
        self.slider.setPageStep(1)
        self.slider.setTickInterval(10)
        self.slider.setTickPosition(Qw.QSlider.TicksBothSides)
        self.slider.setSizePolicy(
            Qw.QSizePolicy.Expanding, Qw.QSizePolicy.Fixed)

        # Link text box and slider
        self.gain_box.valueChanged.connect(lambda x: self.slider.setValue(x * 10))
        self.slider.valueChanged.connect(lambda x: self.gain_box.setValue(x / 10))
        
        # Audio source label
        self.label_text_box = Qw.QLabel(self.label, parent=self)
        self.label_text_box.setAlignment(Qc.Qt.AlignVCenter)
        self.label_text_box.setSizePolicy(
            Qw.QSizePolicy.Fixed, Qw.QSizePolicy.Fixed)

        # Align gain text box and unit label horizontally
        self.inner_layout = Qw.QHBoxLayout()
        self.inner_layout.setContentsMargins(0, 0, 0, 0)
        self.inner_layout.addWidget(self.gain_box)
        self.inner_layout.addWidget(self.unit_label)
        self.inner_layout.setAlignment(Qc.Qt.AlignLeft)

        self.outer_layout = Qw.QVBoxLayout(self)
        self.outer_layout.setContentsMargins(0, 0, 0, 0)
        self.outer_layout.addLayout(self.inner_layout)
        self.outer_layout.addWidget(self.slider)
        self.outer_layout.addWidget(self.label_text_box)
        self.outer_layout.setAlignment(Qc.Qt.AlignVCenter)
        self.setLayout(self.outer_layout)
        self.show()

    def set_gain_no_signal(self, gain: int):
        self.gain_box.blockSignals(True)
        self.gain_box.setValue(gain)
        self.slider.setValue(gain * 10)
        self.gain_box.blockSignals(False)
