
// bling.js https://gist.github.com/paulirish/12fb951a8b893a454b32
window.$ = document.querySelectorAll.bind(document);

Node.prototype.on = window.on = function (name, fn) {
  this.addEventListener(name, fn);
};

NodeList.prototype.__proto__ = Array.prototype;

NodeList.prototype.on = NodeList.prototype.addEventListener = function (name, fn) {
  this.forEach(function (elem, i) {
    elem.on(name, fn);
  });
};


const floydSteinberg = require('floyd-steinberg');

class RemoteDisplay {

	constructor(con) {
		this.con = con;
		this.width = 128;
		this.height = 64;
	}

	drawImage(canvasImageData) {
		const {width, height} = this;
		const imgd = floydSteinberg(canvasImageData);
		const binary = new Uint8Array((imgd.data.length / 4) / 8);
		for (let x = 0; x < width; x++) {
			for (let y = 0; y < height; y++) {
				if (imgd.data[x * 4 + y * width * 4] !== 0) {
					binary[x * Math.floor(height / 8) + Math.floor(y / 8)] |= 1 << (y % 8);
				}
			}
		}
		this.sendCommand(binary);
		return imgd;
	}

	drawPixel({x, y}) {
		this.sendCommand(`d:1:${x}:${y}`);
	}

	clear() {
		this.sendCommand('c');
	}

	sendCommand(data) {
		this.con.send(data);
	}
}

class CanvasDisplay {

	constructor({canvas, remote, scale = 4}, root = document) {
		this.remote = remote;
		this.root = root;
		this.scale = scale;
		this.canvas = canvas;
		this.ctx = canvas.getContext('2d');
		this.isMouseDown = false;
		this.attachEvents();
	}

	attachEvents() {
		this.canvas.addEventListener('mousemove', e => {
			if (this.isMouseDown) {
				this.drawPixel(this.getMousePos(e));
			}
		});

		this.canvas.addEventListener('touchmove', e => {
			e.preventDefault();
			this.drawPixel(this.getTouchPos(e));
		});

		this.canvas.addEventListener('mousedown', e => {
			this.isMouseDown = true;
			this.drawPixel(this.getMousePos(e));
		});

		this.root.addEventListener('mouseup', () => {
			this.isMouseDown = false;
		});
	}

	drawImage(img) {
		const {width, height} = this.canvas;

		this.ctx.clearRect(0, 0, width, height);
		this.ctx.drawImage(img, 0, 0, width, height);

		// Draw on remote
		const imgd = this.remote.drawImage(this.ctx.getImageData(0, 0, width, height));

		this.ctx.putImageData(imgd, 0, 0);
	}

	drawPixel(pos) {
		const {x, y} = pos;
		this.remote.drawPixel(pos);
		this.ctx.fillStyle = '#FFF';
		this.ctx.fillRect(x, y, 1, 1);
	}

	clear() {
		this.remote.clear();
		this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
	}

	getMousePos(evt) {
		const rect = this.canvas.getBoundingClientRect();
		return this.applyScale({
			x: evt.clientX - rect.left,
			y: evt.clientY - rect.top
		});
	}

	getTouchPos(evt) {
		const rect = this.canvas.getBoundingClientRect();
		return this.applyScale({
			x: evt.changedTouches[0].pageX - rect.left,
			y: evt.changedTouches[0].pageY - rect.top
		});
	}

	applyScale({x, y}) {
		return {
			x: Math.floor(x / this.scale),
			y: Math.floor(y / this.scale)
		};
	}

}

const connection = new WebSocket(`ws://${location.host}:81/`);
const remote = new RemoteDisplay(connection);
const localDisplay = new CanvasDisplay({
	canvas: document.querySelector('canvas'),
	scale: 2,
	remote
});

const video = $('video')[0];

let drawToCanvas;
drawToCanvas = () => {
	if (video.paused || video.ended) {
		return false;
	}
	localDisplay.drawImage(video);
	setTimeout(drawToCanvas, 16);
};

video.on('play', () => {
	drawToCanvas();
}, false);

$('#videoLink').on('change', e => {
	video.src = e.target.value;
});

$('#upload').on('change', e => {
	const reader = new FileReader();
	reader.onload = function (event) {
		const img = new Image();
		img.width = 128;
		img.onload = function () {
			localDisplay.drawImage(img);
		};
		img.src = event.target.result;
	};
	reader.readAsDataURL(e.target.files[0]);
});

$('#clear').on('click', () => {
	localDisplay.clear();
});

$('button[data-send]').forEach(b => {
	b.on('click', e => {
		remote.sendCommand(e.target.dataset.send);
	});
});
