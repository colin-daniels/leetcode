#[derive(Copy, Clone, PartialOrd, PartialEq)]
pub struct CircBuf<T: Copy + Default, const N: usize> {
    insertion_index: usize,
    size: usize,
    data: [T; N],
}

impl<T: Copy + Default, const N: usize> Default for CircBuf<T, N> {
    #[inline]
    fn default() -> Self {
        Self::new()
    }
}

impl<T: Copy + Default, const N: usize> CircBuf<T, N> {
    #[inline]
    pub fn new() -> Self {
        Self {
            data: [T::default(); N],
            insertion_index: 0,
            size: 0,
        }
    }

    #[inline]
    pub fn top(&self) -> Option<&T> {
        if self.size != 0 {
            let last_index = (self.insertion_index + N - 1) % N;
            Some(&self.data[last_index])
        } else {
            None
        }
    }

    #[inline]
    pub fn push(&mut self, x: T) -> Option<T> {
        if self.size != N {
            self.data[self.insertion_index] = x;
            self.size = self.insertion_index + 1;
            self.insertion_index = self.size % N;
            None
        } else {
            let old = std::mem::replace(&mut self.data[self.insertion_index], x);
            self.insertion_index = (self.insertion_index + 1) % N;
            Some(old)
        }
    }

    #[inline]
    pub fn iter(&self) -> impl Iterator<Item = &T> {
        let (a, b) = self.data[..self.size].split_at(self.insertion_index);
        b.iter().chain(a.iter()).rev()
    }
}
