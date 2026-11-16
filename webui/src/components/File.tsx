import { DisplayColumns, DisplayRows } from "../services/Display";

export const fileImport = (callback: (frames: boolean[][]) => void) => {
    const csvParser = (file: File, callback: (frames: boolean[][]) => void) => {
        const reader = new FileReader();
        reader.readAsText(file);
        reader.onload = () => {
            const frames: boolean[][] = [];
            let frame: boolean[] = [];
            let y = 0;
            reader.result
                ?.toString()
                .split(/\r?\n/)
                .forEach((row) => {
                    let x = 0;
                    row.split(",").forEach((column) => {
                        const trimmed = column.trim();
                        if (trimmed.length && x < DisplayColumns) {
                            frame.push(trimmed === "true" || parseInt(trimmed, 10) > 0);
                            ++x;
                        }
                    });
                    if (x > 0) {
                        while (x < DisplayColumns) {
                            frame.push(false);
                            ++x;
                        }
                        ++y;
                        if (y >= DisplayRows) {
                            frames.push(frame);
                            frame = [];
                            y = 0;
                        }
                    }
                });
            if (y > 0) {
                while (frame.length < DisplayColumns * DisplayRows) {
                    frame.push(false);
                }
                frames.push(frame);
            }
            if (frames.length) {
                callback(frames);
            }
        };
    };

    const imageParser = (file: File, callback: (frame: boolean[][]) => void) => {
        const reader = new FileReader();
        reader.readAsDataURL(file);
        reader.onload = () => {
            const image = new Image(DisplayColumns, DisplayRows);
            image.src = reader.result as string;
            image.onload = () => {
                const canvas = new OffscreenCanvas(DisplayColumns, DisplayRows);
                const ctx = canvas.getContext("2d");
                ctx?.drawImage(image, 0, 0, DisplayColumns, DisplayRows);
                const imgData = ctx?.getImageData(0, 0, DisplayColumns, DisplayRows);
                if (imgData) {
                    const pixelCount = DisplayColumns * DisplayRows;
                    const luminances = new Uint32Array(pixelCount);
                    let sum = 0;
                    for (let i = 0, p = 0; i < imgData.data.length; i += 4, ++p) {
                        const val = (imgData.data[i] + imgData.data[i + 1] + imgData.data[i + 2]) * imgData.data[i + 3];
                        luminances[p] = val;
                        sum += val;
                    }
                    const avg = sum / pixelCount;
                    callback([
                        Array.from(luminances, (val) => val > avg),
                    ]);
                }
            };
        };
    };

    const upload = document.createElement("input");
    upload.type = "file";
    upload.accept = "text/csv,image/*";
    upload.click();
    upload.onchange = (e: Event) => {
        const input = e.target as HTMLInputElement | null;
        const file = input?.files?.[0];
        if (!file) {
            return;
        }
        file.name.toLowerCase().endsWith(".csv") ? csvParser(file, callback) : imageParser(file, callback);
    };
};

export const csvExport = (name: string, frames: boolean[][]) => {
    let csv = "";
    for (const frame of frames) {
        for (let i = 0; i < frame.length; i += DisplayColumns) {
            csv += `${frame
                .slice(i, i + DisplayColumns)
                .map(Number)
                .join(",")}\r\n`;
        }
    }

    const element = document.createElement("a");
    element.setAttribute("href", `data:text/csv;charset=utf-8,${encodeURIComponent(csv)}`);
    element.setAttribute("download", `${name}.csv`);
    element.style.display = "none";
    document.body.appendChild(element);
    element.click();
    element.remove();
};
