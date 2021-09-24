class DoubleBufferProcessor extends AudioWorkletProcessor {
    constructor(options) {
        super()
        this.frameSize = options.processorOptions.frameSize
        this.currentBuffer = new Float32Array(this.frameSize * options.outputChannelCount[0])
        this.nextBuffer = new Float32Array(this.frameSize * options.outputChannelCount[0])
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
        const channels = outputs[0]
        if (this.index >= this.frameSize) {
            // Currently in underrun.
            if (this.index === this.frameSize) {
                // Only print this once per underrun-streak.
                console.log("Underrun!")
            }
            for (const channel of channels) {
                channel.fill(0)
            }
        } else {
            for (let i = 0; i < channels.length; i++) {
                const start = this.index + this.frameSize * i
                const subarray = this.currentBuffer.subarray(start, start + channels[i].length)
                channels[i].set(subarray)
            }
        }
        // NOTE: This assumes frameSize is a multiple of AudioWorklet frame size (128).
        this.index += channels[0].length
        if (this.index >= this.frameSize && this.nextBufferReady) {
            this.swapBuffers()
        }
        return true
    }
}

registerProcessor("doublebuffer", DoubleBufferProcessor)
