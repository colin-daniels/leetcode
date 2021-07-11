use std::cmp::Ordering;
use std::collections::BinaryHeap;

struct KWIterWrapper<I: Iterator> {
    next: I::Item,
    iter: I,
}

impl<I: Iterator> PartialEq for KWIterWrapper<I>
where
    I::Item: PartialEq,
{
    #[inline]
    fn eq(&self, other: &Self) -> bool {
        self.next.eq(&other.next)
    }
}

impl<I: Iterator> Eq for KWIterWrapper<I> where I::Item: PartialEq {}

impl<I: Iterator> PartialOrd for KWIterWrapper<I>
where
    I::Item: PartialOrd,
{
    #[inline]
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        self.next.partial_cmp(&other.next)
    }
}

impl<I: Iterator> Ord for KWIterWrapper<I>
where
    I::Item: Ord,
{
    #[inline]
    fn cmp(&self, other: &Self) -> Ordering {
        self.next.cmp(&other.next)
    }
}

impl<I: Iterator> KWIterWrapper<I> {
    #[inline]
    fn new(mut iter: I) -> Option<Self> {
        if let Some(next) = iter.next() {
            Some(Self { next, iter })
        } else {
            None
        }
    }

    #[inline]
    fn next(mut self) -> (I::Item, Option<Self>) {
        match self.iter.next() {
            Some(next) => {
                let current = std::mem::replace(&mut self.next, next);
                (current, Some(self))
            }
            None => (self.next, None),
        }
    }
}

pub struct KWayMergeIterator<I: Iterator> {
    min_heap: BinaryHeap<KWIterWrapper<I>>,
}

impl<I: Iterator> KWayMergeIterator<I>
where
    I::Item: Ord,
{
    pub fn new(iterators: impl Iterator<Item = I>) -> Self {
        Self {
            min_heap: iterators.filter_map(KWIterWrapper::new).collect(),
        }
    }
}

impl<I: Iterator> Iterator for KWayMergeIterator<I>
where
    I::Item: Ord,
{
    type Item = I::Item;

    fn next(&mut self) -> Option<Self::Item> {
        match self.min_heap.pop() {
            Some(iter) => {
                let (next, iter) = iter.next();
                if let Some(iter) = iter {
                    self.min_heap.push(iter);
                }

                Some(next)
            }
            None => None,
        }
    }
}
