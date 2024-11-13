/**
 * A helper-class that creates dom and css-api for easy using tooltips anywhere
 * on your site...
 *
 * @author eveter
 */
Tooltip = {
	/**
	 * The starting is here, it must be called once after the definition of this
	 * class.
	 *
	 * @private
	 */
	init: function() {
		this.container = document.createElement('div');
		this.container.id = 'tooltip';
		this.container.className = 'tooltip-container';
		this.container.style.top = '-1000px';
		this.container.style.left = '-1000px';
		this.container.style.position = 'absolute';


		this.titleEl = document.createElement('div');
		this.titleEl.className = 'tooltip-title';
		this.container.appendChild(this.titleEl);

		this.textEl = document.createElement('div');
		this.textEl.className = 'tooltip-body';
		this.container.appendChild(this.textEl);

		document.body.appendChild(this.container);

		if (navigator.userAgent.indexOf('MSIE') > -1) {
			document.attachEvent("onmouseover", Tooltip.mouseOver);
			document.attachEvent("onmouseout", Tooltip.mouseOut);
		} else {
			document.addEventListener("mouseover", Tooltip.mouseOver, false);
			document.addEventListener("mouseout", Tooltip.mouseOut, false);
		}
	},

	mouseOver: function(event) {
		var returnVal = true;
		var t = (event.target) ? event.target : event.srcElement;

		/* handle tooltips */
		if (t.className.indexOf('tooltip') > -1 || t.parentNode.className && t.parentNode.className.indexOf('tooltip') > -1) {
			returnVal = false;
			var tipEl = (t.className.indexOf('tooltip') > -1) ? t : t.parentNode;
			if (tipEl.className.indexOf('::') > -1) {
				var values = tipEl.className.split('::')[1].split(';');
				var optCls = (values.length > 2) ? values[2] : null;
				Tooltip.show(event, values[0], values[1], optCls);
			}
		}

		return returnVal;
	},

	mouseOut: function(event) {
		var returnVal = true;
		var t = (event.target) ? event.target : event.srcElement;

		/* handle tooltips */
		if (t.className.indexOf('tooltip') > -1 || t.parentNode.className && t.parentNode.className.indexOf('tooltip') > -1) {
			returnVal = false;
			Tooltip.hide();
		}

		return returnVal;
	},

	show: function(e, title, text, optCls) {
		this.titleEl.innerHTML = title;
		this.textEl.innerHTML = text;

		var topVal = e.clientY + 15;
		var leftVal = e.clientX + 15;

		if (optCls) {
			this.container.className = 'tooltip-container ' + optCls;
		}

		if ((topVal + this.container.offsetHeight + 10) > document.body.offsetHeight) {
			// the bottom border is overlapping the viewport
			topVal = document.body.offsetHeight - (this.container.offsetHeight + 10);
		}
		if (topVal <= e.clientY) {
			// the top border is overlapping the cursor
			topVal = e.clientY - (this.container.offsetHeight + 10);
		}
		if ((leftVal + this.container.offsetWidth + 10) > document.body.offsetWidth) {
			// the right border is overlapping the viewport
			leftVal = document.body.offsetWidth - (this.container.offsetWidth + 10);
		}

		this.container.style.top = topVal + 'px';
		this.container.style.left = leftVal + 'px';
	},

	hide: function() {
		this.container.style.top = '-1000px';
		this.container.style.left = '-1000px';
		this.container.className = 'tooltip-container';
	},

	reset: function(el) {
		el.onmouseover = null;
	}

};

Tooltip.init();
