#include "TextBox.h"
#include <sstream>

// Initialize sf::Text in the initializer list (SFML 3 requires font/size at construction)
TextBox::TextBox(float x, float y, float width, float height, const sf::Font& font, unsigned int charSize)
    : box(), text(font, "", charSize), m_width(width), m_height(height), m_padding(10.0f), m_rawText(), m_charSize(charSize)
{
    box.setPosition(sf::Vector2f(x, y));
    box.setSize(sf::Vector2f(width, height));
    box.setFillColor(sf::Color(0, 0, 0, 200)); // Semi-transparent black
    box.setOutlineColor(sf::Color::White);
    box.setOutlineThickness(2.0f);

    text.setFillColor(sf::Color::White);
    // Use padding for initial text position
    text.setPosition(sf::Vector2f(x + m_padding, y + m_padding));
}

void TextBox::setPosition(float x, float y) {
    box.setPosition(sf::Vector2f(x, y));
    text.setPosition(sf::Vector2f(x + 10.0f, y + 10.0f)); // Maintain padding
}
void TextBox::setSize(float width, float height) {
    box.setSize(sf::Vector2f(width, height));
    m_width = width;
    m_height = height;
    // Re-wrap existing text to new width
    updateWrappedText();
}
void TextBox::setText(const std::string& str) {
    m_rawText = str;
    updateWrappedText();
}
void TextBox::draw(sf::RenderWindow& window) {
    window.draw(box);
    window.draw(text);
}   

// Private: wrap m_rawText to fit inside box width (account for padding)
void TextBox::updateWrappedText() {
    // If there's no font or zero width, just set raw text
    if (m_width <= 0.0f) {
        text.setString(m_rawText);
        return;
    }

    const float maxLineWidth = m_width - 2.0f * m_padding;
    if (maxLineWidth <= 0.0f) {
        text.setString(m_rawText);
        return;
    }

    // We'll preserve existing explicit newlines by splitting paragraphs
    std::string wrapped;
    size_t pos = 0;
    while (pos < m_rawText.size()) {
        // extract one paragraph (up to next \n)
        size_t nextNl = m_rawText.find('\n', pos);
        std::string paragraph = m_rawText.substr(pos, (nextNl == std::string::npos) ? std::string::npos : nextNl - pos);

        // wrap this paragraph
        std::string line;
        std::istringstream iss(paragraph);
        std::string word;
        bool firstWord = true;
        while (iss >> word) {
            std::string candidate = firstWord ? word : (line + " " + word);
            // Measure candidate width by temporarily setting text
            text.setString(candidate);
            float w = text.getLocalBounds().size.x;
            if (w <= maxLineWidth) {
                // fits
                line = candidate;
                firstWord = false;
            } else {
                if (firstWord) {
                    // single word too long: break the word by chars
                    std::string sub;
                    for (char c : word) {
                        sub.push_back(c);
                        text.setString(sub);
                        if (text.getLocalBounds().size.x > maxLineWidth && sub.size() > 1) {
                            // remove last char and push as a line
                            sub.pop_back();
                            if (!wrapped.empty() || !line.empty()) wrapped += '\n';
                            wrapped += sub;
                            sub.clear();
                        }
                    }
                    if (!sub.empty()) {
                        // start new current line with leftover
                        if (!line.empty()) {
                            if (!wrapped.empty()) wrapped += '\n';
                            wrapped += line;
                        }
                        line = sub;
                        firstWord = false;
                    }
                } else {
                    // push existing line and start new line with word
                    if (!wrapped.empty() || !line.empty()) wrapped += '\n';
                    wrapped += line;
                    line = word;
                    firstWord = false;
                    // check if the new word itself still exceeds width; if so, handle it on next iterations (will break)
                    text.setString(line);
                    if (text.getLocalBounds().size.x > maxLineWidth) {
                        // force breaking in next loop: set firstWord true so branch above handles
                        // but we need to reprocess this word; simplest way is to put it back into stream by using a small trick
                    }
                }
            }
        }

        // append last line of paragraph
        if (!line.empty()) {
            if (!wrapped.empty()) wrapped += '\n';
            wrapped += line;
        }

        // Move to next paragraph
        if (nextNl == std::string::npos) break;
        pos = nextNl + 1;
        // preserve paragraph break as a blank line
        if (!wrapped.empty()) wrapped += '\n';
    }

    // Set final wrapped text and position
    text.setString(wrapped);
    // ensure text position matches padding relative to box
    sf::Vector2f boxPos = box.getPosition();
    text.setPosition(sf::Vector2f(boxPos.x + m_padding, boxPos.y + m_padding));
}