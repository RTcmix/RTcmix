class DoubleBufferProcessor extends AudioWorkletProcessor {
    constructor(options) {
        super()
        this.blockSize = options.processorOptions.blockSize
        this.currentBuffer = new Float32Array(this.blockSize)
        this.nextBuffer = new Float32Array(this.blockSize)
        this.index = 0

        this.port.onmessage = (e) => {
            this.nextBuffer = e.data
            this.nextBufferReady = true
        }

        this.swapBuffers()
    }

    swapBuffers() {
        [this.currentBuffer, this.nextBuffer] = [this.nextBuffer, this.currentBuffer]
        this.index = 0
        this.nextBufferReady = false
        // Send next buffer to fill.
        this.port.postMessage(this.nextBuffer, [this.nextBuffer.buffer])
    }

    process(inputs, outputs, parameters) {
        const output = outputs[0][0]
        if (this.index >= this.blockSize) {
            // Currently in underrun.
            if (this.index === this.blockSize) {
                // Only print this once per underrun-streak.
                console.log("Underrun!")
            }
            output.fill(0)
        } else {
            output.set(this.currentBuffer.subarray(this.index, this.index + output.length))
        }
        // NOTE: This assumes blockSize is a multiple of output.length (128).
        this.index += output.length
        if (this.index >= this.blockSize && this.nextBufferReady) {
            this.swapBuffers()
        }
        return true
    }
}

registerProcessor("doublebuffer", DoubleBufferProcessor)
