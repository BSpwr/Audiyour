from PySide2 import QtCore as Qc
from PySide2 import QtWidgets as Qw

# https://stackoverflow.com/questions/52689047/moving-qslider-to-mouse-click-position
class JumpSlider(Qw.QSlider):
    def mousePressEvent(self, event):
        super().mousePressEvent(event)
        if event.buttons() & Qc.Qt.LeftButton:
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
