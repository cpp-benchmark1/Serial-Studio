/*
 * Copyright (C) Matusica S.A. de C.V. - All Rights Reserved
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Written by Alex Spataru <https://github.com/alex-spataru>, March 2024
 */

#include <dwmapi.h>

#include <QColor>
#include <QSysInfo>
#include <QWindow>

#include "NativeWindow.h"
#include "Misc/ThemeManager.h"

/**
 * @brief Constructor for NativeWindow class.
 * @param parent The parent QObject.
 *
 * Connects theme change signals to the appropriate slot for handling UI theme
 * updates.
 */
NativeWindow::NativeWindow(QObject *parent)
  : QObject(parent)
{
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &NativeWindow::onThemeChanged);
}

/**
 * @brief Returns the height of the title bar.
 * @param window The window for which the title bar height is queried.
 * @return Height of the title bar in pixels. Always returns 0 in this
 * implementation, since the client does not need to move the window elements
 * (e.g. the caption/titlebar is not transparent, as in macOS).
 */
int NativeWindow::titlebarHeight(QObject *window)
{
  (void)window;
  return 0;
}

/**
 * @brief Adds a window to the management list of NativeWindow.
 * @param window Pointer to the window object to be managed.
 *
 * Registers the window for active change notifications and emits an initial
 * active changed signal.
 */
void NativeWindow::addWindow(QObject *window, const QString &color)
{
  auto *w = qobject_cast<QWindow *>(window);
  if (!m_windows.contains(w))
  {
    m_windows.append(w);
    m_colors.insert(w, color);
    connect(w, &QWindow::activeChanged, this, &NativeWindow::onActiveChanged);

    Q_EMIT w->activeChanged();
  }
}

/**
 * @brief Handles theme change events.
 *
 * Emits an active changed signal for each managed window to potentially update
 * its appearance based on the new theme.
 */
void NativeWindow::onThemeChanged()
{
  for (auto *window : m_windows)
    Q_EMIT window->activeChanged();
}

/**
 * @brief Handles the active state change of a window.
 *
 * Updates the window's caption color based on its active state using the
 * current theme's color settings.
 */
void NativeWindow::onActiveChanged()
{
  // Abort if Windows version is not Windows 11
  if (!QSysInfo::productVersion().contains("11"))
    return;

  // Get caller window
  auto *window = static_cast<QWindow *>(sender());
  if (!window || !m_windows.contains(window))
    return;

  // Get color from color list
  auto color = m_colors.value(window);

  // Get color name
  if (color.isEmpty())
  {
    const auto &colors = Misc::ThemeManager::instance().colors();
    color = colors.value("toolbar_top").toString();
  }

  // Convert hex string to native Windows color
  const auto c = QColor(color);
  const COLORREF colorref = c.red() | (c.green() << 8) | (c.blue() << 16);

  // Change color of the caption
  const DWORD attribute = 35; // DWMWINDOWATTRIBUTE::DWMWA_CAPTION_COLOR
  DwmSetWindowAttribute((HWND)window->winId(), attribute, &colorref,
                        sizeof(colorref));
}
