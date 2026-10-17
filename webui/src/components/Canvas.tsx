import { type Component, createEffect, onCleanup, onMount } from "solid-js";

import { Device } from "../config/devices";
import { WebUISidebar } from "../extensions/WebUI";

export const Canvas: Component<{
    disabled?: boolean;
    pixels: boolean[];
    onFrame?: (data: boolean[]) => void;
    onPixel?: (x: number, y: number, lit: boolean) => void;
}> = (props) => {
    let canvasRef: HTMLCanvasElement | undefined;
    let divRef: HTMLDivElement | undefined;
    let isDrawing: boolean = false;
    let lit: boolean = true;

    onMount(() => {
        if (canvasRef) {
            canvasRef.addEventListener("pointerdown", handleDown);
            canvasRef.addEventListener("pointerleave", handleUp);
            canvasRef.addEventListener("pointermove", handleMove);
            canvasRef.addEventListener("pointerup", handleUp);
        }
        if (divRef) {
            const resizeObserver = new ResizeObserver(() => {
                draw();
            });
            resizeObserver.observe(divRef);
            onCleanup(() => {
                resizeObserver.disconnect();
            });
        }
        draw();
    });

    onCleanup(() => {
        canvasRef?.removeEventListener("pointerdown", handleDown);
        canvasRef?.removeEventListener("pointerleave", handleUp);
        canvasRef?.removeEventListener("pointermove", handleMove);
        canvasRef?.removeEventListener("pointerup", handleUp);
    });

    createEffect(() => {
        draw();
    });

    const draw = () => {
        if (!canvasRef || !divRef) {
            return;
        }
        canvasRef.width = divRef.clientWidth;
        canvasRef.height = divRef.clientHeight;
        const columnWidth = canvasRef.width / Device.GRID_COLUMNS;
        const rowHeight = canvasRef.height / Device.GRID_ROWS;
        const pixelHeight =
            rowHeight *
            (Device.LED_SIZE_VERTICAL / Device.PITCH_VERTICAL);
        const pixelWidth =
            columnWidth *
            (Device.LED_SIZE_HORIZONTAL / Device.PITCH_HORIZONTAL);
        const offsetX = (columnWidth - pixelWidth) / 2;
        const offsetY = (rowHeight - pixelHeight) / 2;
        const ctx = canvasRef.getContext("2d");
        if (!ctx) {
            return;
        }
        ctx.clearRect(0, 0, canvasRef.width, canvasRef.height);
        ctx.save();
        ctx.globalAlpha = 1;
        ctx.fillStyle = "black";
        ctx.fillRect(0, 0, canvasRef.width, canvasRef.height);
        for (let x = 0; x < Device.GRID_COLUMNS; ++x) {
            for (let y = 0; y < Device.GRID_ROWS; ++y) {
                ctx.fillStyle = toColor(props.pixels[x + y * Device.GRID_COLUMNS]);
                ctx.fillRect(offsetX + x * columnWidth, offsetY + y * rowHeight, pixelWidth, pixelHeight);
            }
        }
        ctx.restore();
    };

    const toColor = (lit: boolean) => {
        const factor = (lit ? 0xFF : Device.LED_BASE_TONE) / 0xFF;
        return `rgb(${factor * 0xF4},${factor * 0xEB},${factor * 0xDC})`;
    };

    const handleEvent = (e: PointerEvent) => {
        if (canvasRef && !props.disabled) {
            const rect = canvasRef.getBoundingClientRect();
            const _x = Math.floor(
                ((e.clientX - rect.left) * canvasRef.width) / rect.width / (canvasRef.width / Device.GRID_COLUMNS),
            );
            const _y = Math.floor(
                ((e.clientY - rect.top) * canvasRef.height) / rect.height / (canvasRef.height / Device.GRID_ROWS),
            );
            return {
                i: _x + _y * Device.GRID_COLUMNS,
                x: _x,
                y: _y,
            };
        }
        return null;
    };

    const handleDown = (e: PointerEvent) => {
        if (props.disabled) {
            return;
        }
        e.preventDefault();
        const position = handleEvent(e);
        if (!position) {
            return;
        }
        isDrawing = true;
        lit = !props.pixels[position.i];
        props.onPixel?.(position.x, position.y, lit);
        const newState = props.pixels.map((state, index) => (index === position.i ? lit : state));
        props.onFrame?.(newState);
    };

    const handleMove = (e: PointerEvent) => {
        if (!isDrawing || props.disabled) {
            return;
        }
        e.preventDefault();
        const position = handleEvent(e);
        if (!position) {
            return;
        }
        if (props.pixels[position.i] !== lit) {
            props.onPixel?.(position.x, position.y, lit);

            const newState = props.pixels.map((state, index) => (index === position.i ? lit : state));
            props.onFrame?.(newState);
        }
    };

    const handleUp = () => {
        if (isDrawing && props.onFrame) {
            props.onFrame(props.pixels);
        }
        isDrawing = false;
    };

    return (
        <div
            class={`bg-black flex-none inline-block max-h-[calc((100vh-(--spacing(32)))*0.9)] mx-auto p-2.5 relative shrink-0 w-full ${WebUISidebar() ? "max-w-[calc((100vw-(--spacing(80)))*0.9)]" : "max-w-[90vw]"}`}
            ref={(div) => {
                divRef = div;
            }}
            style={{
                "aspect-ratio": `${Device.GRID_COLUMNS * Device.PITCH_HORIZONTAL} / ${Device.GRID_ROWS * Device.PITCH_VERTICAL}`,
            }}
        >
            <canvas
                ref={(canvas) => {
                    canvasRef = canvas;
                }}
            />
        </div>
    );
};
