#pragma once

#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTimeEdit>
#include <QWheelEvent>

#include <type_traits>

namespace score
{
/**
 * @brief The TimeSpinBox class
 *
 * Adapted for the score usage in various duration widgets.
 */
class TimeSpinBox final : public QTimeEdit
{
public:
  TimeSpinBox(QWidget* parent = nullptr) : QTimeEdit(parent)
  {
    setDisplayFormat(QStringLiteral("h.mm.ss.zzz"));
    setAlignment(Qt::AlignRight);
    QPalette palette;
    palette.setColor(QPalette::Highlight, QColor{"#62400a"});
    palette.setColor(QPalette::HighlightedText, QColor{"silver"});
    palette.setColor(QPalette::WindowText, QColor{"#f6a019"});
    palette.setColor(QPalette::Window, QColor{"#62400a"});
    palette.setColor(QPalette::Midlight, QColor{"#62400a"});
    setPalette(palette);
  }

  void wheelEvent(QWheelEvent* event) override { event->ignore(); }
};

template <typename T>
/**
 * @brief The TemplatedSpinBox class
 *
 * Maps a fundamental type to a spinbox type.
 */
struct TemplatedSpinBox;
template <>
struct TemplatedSpinBox<int>
{
  using spinbox_type = QSpinBox;
  using value_type = int;
};
template <>
struct TemplatedSpinBox<float>
{
  using spinbox_type = QDoubleSpinBox;
  using value_type = float;
};
template <>
struct TemplatedSpinBox<double>
{
  using spinbox_type = QDoubleSpinBox;
  using value_type = double;
};

/**
 * @brief The MaxRangeSpinBox class
 *
 * A spinbox mixin that will set its min and max to
 * the biggest positive and negative numbers for the given spinbox type.
 */
template <typename SpinBox>
class MaxRangeSpinBox : public SpinBox::spinbox_type
{
public:
  template <typename... Args>
  MaxRangeSpinBox(Args&&... args)
      : SpinBox::spinbox_type{std::forward<Args>(args)...}
  {
    this->setMinimum(
        std::numeric_limits<typename SpinBox::value_type>::lowest());
    this->setMaximum(std::numeric_limits<typename SpinBox::value_type>::max());
    this->setAlignment(Qt::AlignRight);
  }

  void wheelEvent(QWheelEvent* event) override { event->ignore(); }
};

/**
 * @brief The SpinBox class
 *
 * An abstraction for the most common spinbox type in score.
 */
template <typename T>
class SpinBox final : public MaxRangeSpinBox<TemplatedSpinBox<T>>
{
public:
  using MaxRangeSpinBox<TemplatedSpinBox<T>>::MaxRangeSpinBox;
};

template <>
class SpinBox<double> final : public MaxRangeSpinBox<TemplatedSpinBox<double>>
{
public:
  template <typename... Args>
  SpinBox(Args&&... args) : MaxRangeSpinBox{std::forward<Args>(args)...}
  {
    setDecimals(5);
  }
};
template <>
class SpinBox<float> final : public MaxRangeSpinBox<TemplatedSpinBox<float>>
{
public:
  template <typename... Args>
  SpinBox(Args&&... args) : MaxRangeSpinBox{std::forward<Args>(args)...}
  {
    setDecimals(5);
  }
};

}
