#include "frameless_widget.h"
#include <QAbstractButton>
#include <QApplication>
#include <QCursor>

#ifdef Q_OS_WIN
#include <dwmapi.h>
#include <windows.h>
#include <windowsx.h>
#endif

FramelessWidget::FramelessWidget(QWidget *parent) : QWidget(parent) {
  // 基础属性设置
  setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
  setAttribute(Qt::WA_TranslucentBackground, false);
  setAttribute(Qt::WA_NoSystemBackground);
}

void FramelessWidget::set_drag_bar(QWidget *bar) { drag_bar_ = bar; }

void FramelessWidget::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
#ifdef Q_OS_WIN
  HWND hwnd = (HWND)winId();
  DWORD style = GetWindowLong(hwnd, GWL_STYLE);
  // 恢复 WS_THICKFRAME 以支持阴影和缩放，WS_CAPTION 支持分屏
  SetWindowLong(hwnd, GWL_STYLE,
                style | WS_THICKFRAME | WS_CAPTION | WS_MAXIMIZEBOX |
                    WS_MINIMIZEBOX);
  // 开启 Win11 标准圆角
  DWORD corner = 2;
  DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner,
                        sizeof(corner));
  // 刷新一下边框
  SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
               SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE |
                   SWP_FRAMECHANGED);

#endif
}

bool FramelessWidget::nativeEvent(const QByteArray &eventType, void *message,
                                  qintptr *result) {
#ifdef Q_OS_WIN
  MSG *msg = static_cast<MSG *>(message);
  switch (msg->message) {
  case WM_NCCALCSIZE: {
    *result = 0;
    return true;
  }
  case WM_NCHITTEST: {
    // 1. 获取鼠标位置
    QPoint globalPos = QCursor::pos();
    QPoint localPos = this->mapFromGlobal(globalPos);

    // 2. 边缘缩放检测
    const int border = 8;
    int w = this->width();
    int h = this->height();

    bool isTop = localPos.y() < border;
    bool isBottom = localPos.y() > h - border;
    bool isLeft = localPos.x() < border;
    bool isRight = localPos.x() > w - border;

    if (isTop && isLeft) {
      *result = HTTOPLEFT;
      return true;
    }
    if (isTop && isRight) {
      *result = HTTOPRIGHT;
      return true;
    }
    if (isBottom && isLeft) {
      *result = HTBOTTOMLEFT;
      return true;
    }
    if (isBottom && isRight) {
      *result = HTBOTTOMRIGHT;
      return true;
    }
    if (isTop) {
      *result = HTTOP;
      return true;
    }
    if (isBottom) {
      *result = HTBOTTOM;
      return true;
    }
    if (isLeft) {
      *result = HTLEFT;
      return true;
    }
    if (isRight) {
      *result = HTRIGHT;
      return true;
    }

    // 3. 标题栏拖拽检测 (通用逻辑)
    if (drag_bar_ &&
        drag_bar_->rect().contains(drag_bar_->mapFromGlobal(globalPos))) {
      // 将全局坐标映射到 TitleBar 内部
      QPoint posInTitleBar = drag_bar_->mapFromGlobal(globalPos);

      // 获取鼠标下的子控件
      QWidget *child = drag_bar_->childAt(posInTitleBar);

      // 如果是按钮
      if (qobject_cast<QAbstractButton *>(child)) {
        *result = HTCLIENT; // 让 Qt 处理点击
        return true;
      }

      *result = HTCAPTION; // 否则视为标题栏，允许拖拽
      return true;
    }

    *result = HTCLIENT;
    return true;
  }
  }
#endif
  return QWidget::nativeEvent(eventType, message, result);
}