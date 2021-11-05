use std::io::{self, BufRead};

use graphics::Transformed;
use fontconfig::Fontconfig;
use num_bigint::BigUint;
use opengl_graphics::{Filter, GlyphCache, OpenGL, GlGraphics, Texture, TextureSettings};
use piston::TextEvent;
use piston::event_loop::{Events, EventLoop, EventSettings};
use piston::input::RenderEvent;
use piston::window::WindowSettings;
use piston_window::PistonWindow;
use sdl2_window::Sdl2Window;

mod label;
mod minimap;

const WINDOW_WIDTH: u32 = 640;
const WINDOW_HEIGHT: u32 = 320;

const FONT_SIZE: u32 = 12;

const MINIMAP_HEIGHT: u32 = 64;

fn main() {
    let opengl = OpenGL::V3_2;
    let settings = WindowSettings::new("nsv", [WINDOW_WIDTH, WINDOW_HEIGHT])
        .graphics_api(opengl)
        .exit_on_esc(true);
    // default glutin backend provides annoying dpi handling
    let mut window: PistonWindow<Sdl2Window> = settings.build()
        .expect("Could not create window");

    let fc = Fontconfig::new().unwrap();
    let font = fc.find("monospace", None).unwrap().path;
    let mut glyphs = GlyphCache::new(font, (), TextureSettings::new()).unwrap();

    let sequence: Vec<BigUint> = io::stdin().lock()
        .lines()
        .map(|l| l.unwrap())
        .flat_map(|l| l.split_whitespace().map(|w| w.to_string()).collect::<Vec<_>>())
        .map(|w| w.parse::<BigUint>().unwrap())
        .collect();

    let mut events = Events::new(EventSettings::new().lazy(true));
    let mut gl = GlGraphics::new(opengl);

    let width = sequence.len();
    let height = sequence.iter().map(|x| x.bits()).max().unwrap() as usize;
    let mut buf = vec![0; width * height];

    for (x, number) in sequence.iter().enumerate() {
        for y in 0..height {
            let bitindex = height - y - 1;
            let ison = number.bit(bitindex as u64) as u8;
            buf[x + width * y] = ison * 255;
        }
    }

    let texture_settings = TextureSettings::new().filter(Filter::Nearest);
    let texture = Texture::from_memory_alpha(
        &buf, width as u32, height as u32, &texture_settings
    ).unwrap();

    let mut zoom = 1.0;
    
    let mut xoffset: usize = 0;
    let mut yoffset: usize = 0;

    while let Some(e) = events.next(&mut window) {
        if let Some(args) = e.text_args() {
            match args.as_str() {
                "h" => xoffset = if xoffset > 0 { xoffset - 1 } else { 0 },
                "j" => yoffset = if yoffset > 0 { yoffset - 1 } else { 0 },
                "k" => yoffset = if yoffset < height - 1 { yoffset + 1 } else { 0 },
                "l" => xoffset = if xoffset < width - 1 { xoffset + 1 } else { 0 },

                "-" => zoom = f64::max(1.0 / 8.0, zoom / 2.0),
                "+" => zoom = f64::min(f64::MAX, zoom * 2.0),
                "=" => zoom = 1.0,
                _ => ()
            }
        }

        if let Some(args) = e.render_args() {
            gl.draw(args.viewport(), |c, g| {
                graphics::clear([0.0; 4], g);

                let vpw = c.viewport.unwrap().window_size[0] as usize;
                let vph = c.viewport.unwrap().window_size[1] as usize;

                let main_height = vph - MINIMAP_HEIGHT as usize;

                let x = -1.0 * xoffset as f64;
                let y = -1.0 * (zoom * height as f64 - main_height as f64 - yoffset as f64);

                let transform = c.transform
                    .trans(x, y)
                    .zoom(zoom);

                let main_draw_state = &graphics::draw_state::DrawState::default()
                    .scissor([0, 0, vpw as u32, vph as u32 - MINIMAP_HEIGHT]);

                graphics::Image::new()
                    .rect([0.0, 0.0, width as f64, height as f64])
                    .draw(
                        &texture,
                        main_draw_state,
                        transform,
                        g
                    );
                label::label(
                    format!("x: {}, y: {}", xoffset, yoffset).as_str(),
                    &mut glyphs,
                    FONT_SIZE,
                    main_draw_state,
                    c.transform,
                    g
                );

                let minimap_width = vpw;
                let minimap_height = MINIMAP_HEIGHT;

                minimap::Minimap::new(&sequence).draw(
                    &graphics::draw_state::DrawState::default(),
                    c.transform
                       .trans(0.0, vph as f64 - MINIMAP_HEIGHT as f64)
                       .scale(minimap_width as f64, minimap_height as f64),
                    g
                );
                label::label(
                    "minimap",
                    &mut glyphs,
                    FONT_SIZE,
                    &graphics::draw_state::DrawState::default(),
                    c.transform
                        .trans(0.0, vph as f64 - MINIMAP_HEIGHT as f64),
                    g
                );
            });
        }
    }
}
