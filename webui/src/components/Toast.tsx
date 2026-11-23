import { mdiInformation } from "@mdi/js";
import { createContext, createSignal, type ParentComponent, useContext } from "solid-js";

import { Icon } from "./Icon";

type ToastItem = {
  id: number;
  text: string;
};

const [getToast, setToast] = createSignal<ToastItem | null>(null);

const toast = (text: string, duration: number = 3e3) => {
  const id = Math.random();
  setToast({
    id,
    text,
  });
  setTimeout(() => {
    if (getToast()?.id === id) {
      setToast(null);
    }
  }, duration);
};

const ToastContext = createContext<{
  toast: (text: string, duration?: number) => void;
}>({
  toast,
});

export const ToastProvider: ParentComponent = (props) => {
  const Toast = (props: { item: ToastItem }) => {
    return (
      <button
        class="border bg-base-light dark:bg-base-dark cursor-pointer flex items-center px-4 py-3 rounded-md shadow-lg space-x-2"
        onClick={() => setToast(null)}
        style="animation: toast 0.15s ease-out forwards;"
        type="button"
      >
        <Icon class="mr-2" path={mdiInformation} />
        {props.item.text}
      </button>
    );
  };

  return (
    <ToastContext
      value={{
        toast,
      }}
    >
      <div class="bottom-4 fixed left-4 z-50" role="status">
        {(() => {
          const item = getToast();
          return item && <Toast item={item} />;
        })()}
      </div>
      {props.children}
    </ToastContext>
  );
};

export const Toast = () => useContext(ToastContext);
