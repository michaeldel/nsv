use num_bigint::BigUint;
use piston_window::{Graphics, Transformed};

pub struct Minimap<'a> {
    sequence: &'a Vec<BigUint>
}

impl<'a> Minimap<'a> {
    pub fn new(sequence: &'a Vec<BigUint>) -> Self {
        Self { sequence }
    }
}

impl Minimap<'_> {
    pub fn draw<G>(
        &self,
        draw_state: &graphics::DrawState,
        transform: graphics::math::Matrix2d,
        g: &mut G
    ) where G: Graphics {
        let color = [1.0, 1.0, 1.0, 1.0]; 
        let radius = 1.0 / self.sequence.len() as f64;

        let max_bits = self.sequence.iter().map(|x| x.bits()).max().unwrap();

        for (i, num) in self.sequence.iter().enumerate() {
            let x = i as f64 / self.sequence.len() as f64;
            let h = num.bits() as f64 / max_bits as f64;

            debug_assert!(0.0 <= x && x <= 1.0);
            debug_assert!(0.0 <= h && h <= 1.0);

            graphics::Line::new(color, radius).draw(
                [x, 0.0, x, h],
                draw_state,
                transform.trans(0.0, 1.0).flip_v(),
                g
            );
        }
    }
}
