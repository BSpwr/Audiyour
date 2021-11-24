from PySide2 import QtCore as Qc
from PySide2 import QtWidgets as Qw
from PySide2 import QtGui as Qg
import asyncio

class Equalizer(Qw.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.freq_bands = [31, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000]
        self.band_widgets = []
        
        self.set_enable_btn = Qw.QPushButton("Enable equalizer")
        self.set_enable_btn.setCheckable(True)
        self.set_enable_btn.toggled.connect(self.set_equalizer_enable)
        
        self.set_defaults_btn = Qw.QPushButton("Set defaults")
        self.set_defaults_btn.clicked.connect(self.set_defaults)
        
        self.load_settings_btn = Qw.QPushButton("Load from Device")
        self.load_settings_btn.clicked.connect(lambda: asyncio.ensure_future(self.load_settings()))
        
        self.toolbar_layout = Qw.QHBoxLayout()
        self.toolbar_layout.addWidget(self.set_enable_btn)
        self.toolbar_layout.addWidget(self.set_defaults_btn)
        self.toolbar_layout.addWidget(self.load_settings_btn)
        
        self.bands_layout = Qw.QHBoxLayout()
    
        for idx, b_freq in enumerate(self.freq_bands):
            if b_freq > 999:
                band_w = EqualizerBand(f'{int(b_freq / 1000) if b_freq % 1000 == 0 else (b_freq / 1000)} kHz', parent=self)
            else:
                band_w = EqualizerBand(f'{b_freq} Hz', parent=self)
                
            band_w.slider.valueChanged.connect(lambda x, idx=idx: asyncio.ensure_future(self.do_band_update(idx, dB=x)))

            self.band_widgets.append(band_w)
            self.bands_layout.addWidget(band_w)
        
        self.layout = Qw.QVBoxLayout()
        self.layout.addLayout(self.toolbar_layout)
        self.layout.addLayout(self.bands_layout)
            
        self.setLayout(self.layout)
        
        self.show()
    
    async def load_settings(self):
        await self.parent().parent().bt_man.read_eq_gains()
        gains = self.parent().parent().bt_man.eq_gains
        # status = self.parent().parent().parent().ser_man.get_eq_status()

        for idx, band_w in enumerate(self.band_widgets):
            band_w.slider.blockSignals(True)
            band_w.slider.setValue(int(gains[idx]))
            band_w.update_db_value(int(gains[idx]))
            band_w.slider.blockSignals(False)
        # self.set_enable_btn.setChecked(status)
    
    def set_equalizer_enable(self):
        if self.set_enable_btn.isChecked():
            self.set_enable_btn.setText("Disable equalizer")
            # self.parent().parent().parent().ser_man.set_eq_en(True)
        else:
            self.set_enable_btn.setText("Enable equalizer")
            # self.parent().parent().parent().ser_man.set_eq_en(False)
    
    def set_defaults(self):
        for band_w in self.band_widgets:
            band_w.slider.blockSignals(True)
            band_w.slider.setValue(0)
            band_w.update_db_value(0)
            band_w.slider.blockSignals(False)

        asyncio.ensure_future(self.parent().parent().bt_man.write_gains([0 for i in range(0, 10)]))

    
    async def do_band_update(self, band_num, dB):
        # self.parent().parent().parent().ser_man.set_eq_band(band_num, dB)
        
        await self.parent().parent().bt_man.write_gain_index(band_num, int(dB))
        self.parent().parent().update_conn_status()
        
class EqualizerBand(Qw.QWidget):
    def __init__(self, center_freq: str, parent=None):
        super().__init__(parent)
        self.center_freq = center_freq
        
        self.slider = JumpSlider(Qc.Qt.Vertical, parent=self)
        self.slider.setMinimum(-20)
        self.slider.setMaximum(12)
        self.slider.setSingleStep(1)
        self.slider.setPageStep(1)
        self.slider.setTickInterval(1)
        self.slider.setTickPosition(Qw.QSlider.TicksBothSides)
        self.slider.setSizePolicy(
            Qw.QSizePolicy.Fixed, Qw.QSizePolicy.Expanding)
        self.slider.valueChanged.connect(lambda val: self.update_db_value(val))
        
        self.db_value_label = Qw.QLabel("0 dB")
        self.db_value_label.setFixedWidth(60)
        self.db_value_label.setAlignment(Qc.Qt.AlignVCenter)
        self.db_value_label.setSizePolicy(
            Qw.QSizePolicy.Fixed, Qw.QSizePolicy.Fixed)

        self.freq_text_box = Qw.QLabel(self.center_freq, parent=self)
        self.freq_text_box.setAlignment(Qc.Qt.AlignVCenter)
        self.freq_text_box.setSizePolicy(
            Qw.QSizePolicy.Fixed, Qw.QSizePolicy.Fixed)

        self.layout = Qw.QVBoxLayout()
        self.layout.addWidget(self.db_value_label, Qc.Qt.AlignVCenter)
        self.layout.addWidget(self.slider, Qc.Qt.AlignVCenter)
        self.layout.addWidget(self.freq_text_box, Qc.Qt.AlignVCenter)
        self.layout.setAlignment(Qc.Qt.AlignVCenter)
        self.setLayout(self.layout)
        self.show()
        
    def update_db_value(self, value: int):
        self.db_value_label.setText(f'{value} dB')

# class DiffSurround(Qw.QWidget):
#     def __init__(self, parent=None):
#         super().__init__(parent)
        
#         self.slider = JumpSlider(Qc.Qt.Horizontal, parent=self)
#         self.slider.setMinimum(0)
#         self.slider.setMaximum(500)
#         self.slider.setSingleStep(1)
#         self.slider.setPageStep(1)
#         self.slider.setTickInterval(10)
#         self.slider.setTickPosition(Qw.QSlider.TicksBothSides)
#         self.slider.setSizePolicy(
#             Qw.QSizePolicy.Expanding, Qw.QSizePolicy.Fixed)
#         self.slider.valueChanged.connect(lambda val: self.do_diff_surround_update(val / 10))
#         self.slider.valueChanged.connect(lambda val: self.delay_ms_label.setText(f'{round(val / 10, 1)} ms'))
        
#         self.delay_ms_label = Qw.QLabel("0.0 ms")
#         self.delay_ms_label.setFixedWidth(50)
        
#         self.layout = Qw.QHBoxLayout()
#         self.layout.addWidget(self.slider)
#         self.layout.addWidget(self.delay_ms_label)
        
#         self.setLayout(self.layout)
#         self.show()
        
#     def load_settings(self):
#         # delay_ms = self.parent().parent().parent().ser_man.get_diff_surround_ms()
#         # self.slider.setValue(int(delay_ms * 10))
#         pass
        
#     def do_diff_surround_update(self, delay_ms):
#         # self.parent().parent().parent().ser_man.set_diff_surround_ms(delay_ms)
#         pass

# class FixedLabel(Qw.QLabel):
#     def __init__(self, text, max_text, parent=None:)

# https://stackoverflow.com/questions/52689047/moving-qslider-to-mouse-click-position
# https://stackoverflow.com/questions/46147290/pyqt5-check-if-mouse-is-held-down-in-enter-event
class JumpSlider(Qw.QSlider):
    def __init__(self, orientation, parent=None):
        super().__init__(orientation, parent)
        self.setAcceptDrops(True)
        self.dragstart = None
        
    def mousePressEvent(self, event):
        super().mousePressEvent(event)
        if event.buttons() & Qc.Qt.LeftButton:
            val = self.pixelPosToRangeValue(event.pos())
            self.setValue(val)
            self.dragstart = event.pos()
    
    def mouseReleaseEvent(self, event):
        self.dragstart = None

    def mouseMoveEvent(self, event):
        super().mouseMoveEvent(event)
        start_drag_distance = 60
        if self.orientation() == Qc.Qt.Horizontal:
            if (self.dragstart is not None and
                event.buttons() & Qc.Qt.LeftButton and
                abs((event.pos() - self.dragstart).y()) > start_drag_distance):
                self.dragstart = None
                drag = Qg.QDrag(self)
                drag.setMimeData(Qc.QMimeData())
                drag.exec_(Qc.Qt.LinkAction)
        else:
            if (self.dragstart is not None and
            event.buttons() & Qc.Qt.LeftButton and
            abs((event.pos() - self.dragstart).x()) > start_drag_distance):
                self.dragstart = None
                drag = Qg.QDrag(self)
                drag.setMimeData(Qc.QMimeData())
                drag.exec_(Qc.Qt.LinkAction)
            
    def dragEnterEvent(self, event):
        event.acceptProposedAction()
        val = self.pixelPosToRangeValue(event.pos())
        self.setValue(val)

    def pixelPosToRangeValue(self, pos):
        opt = Qw.QStyleOptionSlider()
        self.initStyleOption(opt)
        gr = self.style().subControlRect(Qw.QStyle.CC_Slider, opt, Qw.QStyle.SC_SliderGroove, self)
        sr = self.style().subControlRect(Qw.QStyle.CC_Slider, opt, Qw.QStyle.SC_SliderHandle, self)

        if self.orientation() == Qc.Qt.Horizontal:
            sliderLength = sr.width()
            sliderMin = gr.x()
            sliderMax = gr.right() - sliderLength + 1
        else:
            sliderLength = sr.height()
            sliderMin = gr.y()
            sliderMax = gr.bottom() - sliderLength + 1
        pr = pos - sr.center() + sr.topLeft()
        p = pr.x() if self.orientation() == Qc.Qt.Horizontal else pr.y()
        return Qw.QStyle.sliderValueFromPosition(self.minimum(), self.maximum(), p - sliderMin,
                                               sliderMax - sliderMin, opt.upsideDown)