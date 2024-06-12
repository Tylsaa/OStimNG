
import com.greensock.*;
import gfx.ui.InputDetails;
import Shared.GlobalFunc;
import gfx.ui.NavigationCode;
import gfx.managers.FocusHandler;
import gfx.events.EventDispatcher;

class SearchBar extends MovieClip
{
	var bg:MovieClip;
	var divider:MovieClip;
	var textInput:MovieClip;
	var fakeButton:MovieClip;

	var CurrentlySelectedIdx = -1;
	var topGutter = 10;
	var bottomGutter = 28;
	var optionGutter = 8;
	var lineHeight = 48;
	var minHeight = lineHeight + optionGutter + bottomGutter;
	var maxOptions = 10;
	var menuGutter = 22;
	var _isOpen = false;
	var scrollPosition:Number = 0;
	var visibleItems:Array = [];
	var maxScrollPosition:Number;
	var fields = new Array();
	var PrevFocus:MovieClip;
	var _inputtingText:Boolean = false;


	var widthTweenTime:Number = 0.25;
	var heightTweenTime:Number = 0.25;

     private var mouseWheelListener:Object;
     {
	public function SearchBar()
     {
        super();
        init();
     }
     private function init():Void 
     {
		bg._width = 0;
		bg._height = 0;
		UpdateSize(minHeight,355);

		mouseWheelListener = new Object();
        	mouseWheelListener.onMouseWheel = Delegate.create(this, onMouseWheel);
        	Mouse.addListener(mouseWheelListener);

		// For Testing
		//AssignData(generateTestData(5));
     }
	
     private function onMouseWheel(delta:Number):Void 
     {
        if (delta > 0) {
            scrollUp();
     } else if (delta < 0) {
            scrollDown();
     }
     }
     private function scrollUp():Void {
        if (scrollPosition > 0) {
            	scrollPosition--;
            	updateVisibleItems();
     }
     }

     private function scrollDown():Void {
    	if (scrollPosition < maxScrollPosition) {
        	scrollPosition++;
        	updateVisibleItems();
     }
     }

     public function AssignData(data:Array):Void
     	if (!data || data.length === 0) {
		clearData();
     }
    
     visibleItems = [];
    
     var totalHeight:Number = minHeight + topGutter + ((optionGutter + lineHeight) * data.length);
     var maxVisibleItems:Number = Math.floor((bg._height - topGutter) / (optionGutter + lineHeight));
    
     totalHeight = Math.max(totalHeight, minHeight);
     maxScrollPosition = data.length - maxVisibleItems;
    
     scrollPosition = Math.max(0, Math.min(scrollPosition, maxScrollPosition));
    
     visibleItems = data.slice(scrollPosition, scrollPosition + maxVisibleItems);
     updatevisibleItems();
     }
     private function updateVisibleItems():Void
     {
     for (var i:Number = 0; i < fields.length; i++)
     {
        if (!fields[i])
     {
        	fields[i] = this.attachMovie("SearchOption", "o" + i, i + 1, {_x: menuGutter, _y: 0 - (minHeight + lineHeight + ((optionGutter + lineHeight) * i)), _height: lineHeight});
     }      
			fields[i]._visible = true;
            		fields[i].init(i, visibleItems[i]);
     }
     }
     public function UpdateSize(newHeight:Number, newWidth:Number):Void
     {
		TweenLite.to(bg,widthTweenTime,{_width:newWidth + (menuGutter * 2), _x:(newWidth + (menuGutter * 2)) / 2});
		TweenLite.to(divider,0.5,{_width:newWidth, _x:menuGutter});
		TweenLite.to(bg,heightTweenTime,{delay:widthTweenTime, _height:newHeight, _y:0 - (newHeight / 2)});
     }
     }

     private function debounce(func:Function, delay:Number):Function {
     var timer:Number;
     return function() {
        clearTimeout(timer);
        var context = this, args = arguments;
        timer = setTimeout(function() {
            func.apply(context, args);
        }, delay);
     };
     }
     public function removeMouseWheelListener():Void {
        if (mouseWheelListener != null) {
            Mouse.removeListener(mouseWheelListener);
            mouseWheelListener = null;
     }
     }
     public function onDestroy():Void {
        removeMouseWheelListener();
     }
     public function HandleKeyboardInput(input:Number):Void
	{
     switch (input)
     {
        case 0:
            debounce(scrollUp(), 100).call(this);
            break;
        case 1:
            debounce(scrollDown(), 100).call(this);
            break;
     }
     }
     public function handleInput(details:InputDetails, pathToFocus:Array):Boolean
     {
		if (GlobalFunc.IsKeyPressed(details))
		{
			if (details.navEquivalent == NavigationCode.ENTER && details.code != 32)
			{
				if (inputtingText)
				{
					Search();
					ClearInput();
					DisableTextInput();
					return true;
				}
			}
			else if (details.navEquivalent == NavigationCode.TAB || details.navEquivalent == NavigationCode.ESCAPE)
			{
				ClearAndHide();
				return true;
     }
     }
			return false;
     }
     private function onMouseDown():Void
     {
		if (Mouse.getTopMostEntity() == textInput.textField)
		{
			EnableTextInput();
		}
     }

     private function UpdateHighlight():Void
     {
		for (var i = 0; i < fields.length; i++)
		{
			fields[i].OnUnHighlight();
		}
		if (CurrentlySelectedIdx != -1)
		{
			fields[CurrentlySelectedIdx].OnHighlight();
		}

     }

     private function ClearAndHide():Void
     {
		doHideMenuRequest();
     }
     private function ClearInput():Void
     {
		textInput.text = "";
     }
     private function DisableTextInput():Void
     {
		inputtingText = false;
		textInput.editable = false;
		gfx.managers.FocusHandler.instance.setFocus(fakeButton,0);
		Selection.setFocus(fakeButton);
     }
     private function EnableTextInput():Void
     {
		textInput.editable = true;
		gfx.managers.FocusHandler.instance.setFocus(textInput.textField,0);
		Selection.setFocus(textInput.textField);
		inputtingText = true;
     }

	
     private function clearData():Void {
        for (var k = 0; k < fields.length; k++) {
            fields[k]._visible = false;
            fields[k].optionVal.text = "";
     }
        HideDivider();
        var newHeight = minHeight;
        TweenLite.to(bg, 0.5, {_height: newHeight, _y: 0 - (newHeight / 2), _width: 355, _x: 355 / 2});
        CurrentlySelectedIdx = -1;
        EnableTextInput();
     }

     public function SetIdx(field):Void {
        CurrentlySelectedIdx = field.idx;
     }
     public function SetIsOpen(isOpen:Boolean)
     {
		this._isOpen = isOpen;
		if (_isOpen)
		{
			EnableTextInput();
			ClearInput();
		}
		else
		{
			ClearInput();
			AssignData([]);
			DisableTextInput();
		}
     }

     public function SelectOption()
     {
		var val = fields[CurrentlySelectedIdx].sceneid;
		if (val != undefined)
		{
			doSelectOption(val);
			ClearAndHide();
		}
     }


     private function HideDivider():Void
     {
		divider._alpha = 00;
		divider._visible = false;
     }

     private function ShowDivider():Void
     {
		divider._visible = true;
		divider._alpha = 70;
     }

     private function generateTestData(count:Number):Array
     {
		_isOpen = true;
		var arr = new Array(count);
		for (var i = 0; i < count; i++)
		{
			arr[i] = {label:"AAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBB" + i, value:"v" + i};
		}
		return arr;
     }

     private function Search():Void
     {
		doSearch(textInput.text);
     }

     public function doSelectOption(val:String):Void
     {

     }

     private function doHideMenuRequest():Void
     {

     }

     private function doSearch(val:String):Void
     {

     }

     private function doSetInputtingText(inputting:Boolean):Void
     {

     }

}
