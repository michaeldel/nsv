use std::io::{self, BufRead};

use graphics::Transformed;
use num_bigint::BigUint;
use opengl_graphics::{Filter, OpenGL, GlGraphics, Texture, TextureSettings};
use piston::TextEvent;
use piston::event_loop::{Events, EventLoop, EventSettings};
use piston::input::RenderEvent;
use piston::window::WindowSettings;
use piston_window::PistonWindow;
use sdl2_window::Sdl2Window;

const WINDOW_WIDTH: u32 = 640;
const WINDOW_HEIGHT: u32 = 320;

fn main() {
    let opengl = OpenGL::V3_2;
    let settings = WindowSettings::new("nsv", [WINDOW_WIDTH, WINDOW_HEIGHT])
        .graphics_api(opengl)
        .exit_on_esc(true);
    // default glutin backend provides annoying dpi handling
    let mut window: PistonWindow<Sdl2Window> = settings.build()
        .expect("Could not create window");

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

                let vph = c.viewport.unwrap().window_size[1] as usize;

                let x = -1.0 * xoffset as f64;
                let y = -1.0 * (zoom * height as f64 - vph as f64 - yoffset as f64);

                let transform = c.transform
                    .trans(x, y)
                    .zoom(zoom);

                graphics::Image::new()
                    .rect([0.0, 0.0, width as f64, height as f64])
                    .draw(
                        &texture,
                        &graphics::draw_state::DrawState::default(),
                        transform,
                        g
                    );
            });
        }
    }
}
