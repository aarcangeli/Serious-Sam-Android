package com.github.aarcangeli.serioussamandroid.lists;

import java.util.List;

public class ButtonSet
{
	public List<ButtonSet> ButtonSet;
	public String bitmap, type, action;
	public int h, w;
	public float x, y;
	public ButtonSet(String buttonType_, float x_, float y_, int h_, int w_, String bitmap_, String keycode_) { 
		type = buttonType_;
		x = x_;
		y = y_;
		h = h_;
		w = w_;
		bitmap = bitmap_;
		action = keycode_;
	}
	
	public List<ButtonSet> getButtonSet() {
		return ButtonSet;
	}

	public void setButtonSet(List<ButtonSet> ButtonSet) {
		this.ButtonSet = ButtonSet;
	}  
}