/*
    Big ComBoy
    Copyright (C) 2023-2024 UltimaOmega474

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "Style.hpp"
#include <QApplication>
#include <QPalette>
#include <QStyleFactory>

namespace QtFrontend
{
    StyleColor StyleColor::DefaultDark()
    {
        auto main = QColor::fromString("#282828");
        auto text = QColor::fromString("#ffffff");

        return StyleColor{
            .main = main,
            .base = main.darker(350),
            .highlight = QColor::fromString("#218aff").darker(150),
            .text = text,
            .disabled_text = text.darker(),
        };
    }

    void ApplyColorPalette(const StyleColor &style_color)
    {
        qApp->setStyle(QStyleFactory::create("fusion"));
        QPalette palette;
        palette.setColor(QPalette::Window, style_color.main);
        palette.setColor(QPalette::WindowText, style_color.text);
        palette.setColor(QPalette::Base, style_color.base);
        palette.setColor(QPalette::AlternateBase, style_color.main);
        palette.setColor(QPalette::ToolTipText, style_color.text);
        palette.setColor(QPalette::Text, style_color.text);
        palette.setColor(QPalette::Button, style_color.main);
        palette.setColor(QPalette::ButtonText, style_color.text);
        palette.setColor(QPalette::Highlight, style_color.highlight);
        palette.setColor(QPalette::HighlightedText, style_color.text);

        palette.setColor(QPalette::Disabled, QPalette::ButtonText, style_color.disabled_text);
        palette.setColor(QPalette::Disabled, QPalette::WindowText, style_color.disabled_text);
        palette.setColor(QPalette::Disabled, QPalette::Text, style_color.disabled_text);
        qApp->setPalette(palette);
    }
}
