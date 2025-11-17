#ifndef TextBox_H
#define TextBox_H

#include <SFML/Graphics.hpp>
#include <string>

class TextBox {
public:
    TextBox(float x, float y, float width, float height, const sf::Font& font, unsigned int charSize = 16);
    void setPosition(float x, float y);
    void setSize(float width, float height);
    void setText(const std::string& text);
    void draw(sf::RenderWindow& window);
private:
    sf::RectangleShape box;
    sf::Text text;
    // Wrapping state
    float m_width;
    float m_height;
    float m_padding = 10.0f;
    std::string m_rawText; // unwrapped input text
    unsigned int m_charSize;
    // Recompute wrapped text from m_rawText into `text` using current size/font
    void updateWrappedText();
};

#endif // TextBox_H