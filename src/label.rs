use graphics::character::CharacterCache;
use opengl_graphics::GlyphCache;
use piston_window::{Graphics, Transformed};

pub fn label<G>(
    text: &str,
    glyphs: &mut GlyphCache,
    font_size: u32,
    draw_state: &graphics::DrawState,
    transform: graphics::math::Matrix2d,
    g: &mut G
) where G: Graphics<Texture = opengl_graphics::Texture> {
    let mut width = 0.0;
    let mut height = 0.0;

    for ch in text.chars() {
        let character = glyphs.character(font_size, ch).ok().unwrap();

        width += character.advance_width() + character.left();
        height = (character.advance_height() + character.top()).max(height);
    }

    let background_color = [0.0, 0.0, 0.0, 0.8];
    let padding = 4.0;

    width += 2.0 * padding;
    height += 2.0 * padding;

    graphics::Rectangle::new(background_color).draw(
        [0.0, 0.0, width as f64, height as f64],
        draw_state,
        transform,
        g
    );
    graphics::text::Text::new_color([1.0, 1.0, 1.0, 1.0], font_size).draw(
        text,
        glyphs,
        draw_state,
        transform.trans(padding, height - font_size as f64 / 2.0),
        g
    );
}
