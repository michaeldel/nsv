use std::io::{self, BufRead};

use glutin_window::GlutinWindow;
use graphics::Transformed;
use num_bigint::BigUint;
use opengl_graphics::{Filter, OpenGL, GlGraphics, Texture, TextureSettings};
use piston::event_loop::{Events, EventLoop, EventSettings};
use piston::input::RenderEvent;
use piston::window::WindowSettings;

const WINDOW_WIDTH: u32 = 640;
const WINDOW_HEIGHT: u32 = 320;

fn main() {
    let opengl = OpenGL::V3_2;
    let settings = WindowSettings::new("nsv", [WINDOW_WIDTH, WINDOW_HEIGHT])
        .graphics_api(opengl)
        .exit_on_esc(true);
    let mut window: GlutinWindow = settings.build()
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

    let image = graphics::Image::new().rect([0f64, 0f64, width as f64, height as f64]);
    let texture_settings = TextureSettings::new().filter(Filter::Nearest);
    let texture = Texture::from_memory_alpha(
        &buf, width as u32, height as u32, &texture_settings
    ).unwrap();

    let zoom = 4.0;

    while let Some(e) = events.next(&mut window) {
        if let Some(args) = e.render_args() {
            gl.draw(args.viewport(), |c, g| {
                graphics::clear([0.0; 4], g);
                let zoomed = c.transform.zoom(zoom);
                image.draw(
                    &texture, &graphics::draw_state::DrawState::default(), zoomed, g
                );
            });
        }
    }
}
