import { useCallback, useEffect, useState } from 'react';

const up = {
  c: 0,
  fn: () => {
    up.c += 1;
    up.notify();
  },
  subscribers: new Set<() => void>(),
  notify: () => {
    up.subscribers.forEach((fn) => fn());
  },
};

export function useCounter() {
  const [count, setCount] = useState(up.c);

  useEffect(() => {
    const uppp = () => {
      setCount(up.c);
    };
    up.subscribers.add(uppp);

    return () => {
      up.subscribers.delete(uppp);
    };
  }, []);

  const increment = useCallback(() => {
    up.fn();
  }, []);

  return { count, increment };
}
