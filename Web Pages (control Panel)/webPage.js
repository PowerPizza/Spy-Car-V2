function autoResize(){
    let width_ = document.documentElement.clientWidth;
    if (width_ < 930){
        document.body.style.gridTemplateColumns = "none";
        document.body.style.gridTemplateRows = "1fr 1fr";
    }
    else{
        document.body.style.gridTemplateColumns = "";
        document.body.style.gridTemplateRows = "";
    }
}
window.onresize = autoResize;
window.onload = autoResize;

function sendCommand(data){
    let xmlServer = new XMLHttpRequest();
    xmlServer.open("GET", `/${data}`);
    xmlServer.onload = ()=>{
        console.log(xmlServer.responseText);
    }
    xmlServer.send();
}

let dirY = null;
let dirX = null;
let lastButtonChange = null;
function onclickArrowBtns(id){
    if (dirX){
        dirX.style.transform = "";
        dirX.style.backgroundColor = "";
    };
    if (dirY){
        dirY.style.transform = "";
        dirY.style.backgroundColor = "";
    };

    if (id == "symbols_arrow_left" || id == "symbols_arrow_right"){
        dirX = document.getElementById(id);
    }
    else if (id == "symbols_arrow_up" || id == "symbols_arrow_down"){
        dirY = document.getElementById(id);
    }
    else{
        dirX = document.getElementById(id);
        dirY = document.getElementById(id);
    }
    document.getElementById(id).style.transform = "scale(1.08)";
    document.getElementById(id).style.backgroundColor = "lime";

    sendCommand(id);
}

// camera x y cordinates gui...
class range_slider{
    constructor(min=0, max=100, value=50, parent=document.body, on_slide=(val)=>{return val;}){
        this.value = value;
        this.max = max;
        this.min = min;
        this.parent = parent;
        this.on_slide = on_slide;
        this.is_used = false;
    }
    
    createVerticalRS(){
        if (this.is_used){return;}
        this.is_used = true;

        let slider_bar = document.createElement("div");
        slider_bar.className = "slider_bar_vr";

        let slider = document.createElement("span");
        slider.className = "slider";

        var main_this = this;
        
        class pos_control{
            constructor(pos_Y, func_call) {
                if (pos_Y < main_this.min || pos_Y > slider_bar.clientHeight){
                    return;
                }
                else if (pos_Y <= slider.clientHeight) {
                    slider.style.top = 0 + "px";
                }
                else if (pos_Y >= slider_bar.clientHeight) {
                    slider.style.top = slider_bar.clientHeight - slider.clientHeight + "px";
                }
                else {
                    slider.style.top = pos_Y - slider.clientHeight + "px";
                }
                main_this.value =  main_this.max+1 - Math.round(pos_Y / (slider_bar.clientHeight / main_this.max));
                slider.innerText = main_this.value;
                if (func_call){ main_this.on_slide(main_this.value); }
            }
        }

        slider_bar.addEventListener("click", (eve)=>{new pos_control(eve.clientY - slider_bar.getBoundingClientRect().top, true);});
    
        slider_bar.addEventListener("mousemove", (eve)=>{
            if (eve.clientY != 0 && eve.buttons == 1){
                new pos_control(eve.clientY - slider_bar.getBoundingClientRect().top, false);
            }
        }, true);

        slider_bar.addEventListener("touchmove", (eve)=>{
            new pos_control(eve.changedTouches[0].clientY - slider_bar.getBoundingClientRect().top, true);
        }, true)
        
        slider_bar.appendChild(slider);
        this.parent.appendChild(slider_bar);

        new pos_control((this.value * slider_bar.clientHeight) / this.max, true);
    }

    createHorizontalRS(){
        if (this.is_used){return;}
        this.is_used = true;

        let slider_bar = document.createElement("div");
        slider_bar.className = "slider_bar_hr";

        let slider = document.createElement("span");
        slider.className = "slider";
        
        let val_viewer = document.createElement("p");
        val_viewer.className = "val_viewer";
        slider.appendChild(val_viewer);

        var main_this = this;

        class pos_control{
            constructor(pos_X, func_call) {
                if (pos_X < main_this.min || pos_X > slider_bar.clientWidth){
                    return;
                }
                else if (pos_X <= slider.clientWidth) {
                    slider.style.left = 0 + "px";
                }
                else if (pos_X >= slider_bar.clientWidth) {
                    slider.style.left = slider_bar.clientWidth - slider.clientWidth + "px";
                }
                else {
                    slider.style.left = pos_X - slider.clientWidth + "px";
                }
                main_this.value = Math.round(pos_X / (slider_bar.clientWidth / main_this.max));
                val_viewer.innerText = main_this.value;
                if (func_call){ main_this.on_slide(main_this.value); }
            }
        }

        slider_bar.addEventListener("click", (eve)=>{new pos_control(eve.clientX - slider_bar.getBoundingClientRect().left, true);});
    
        slider_bar.addEventListener("mousemove", (eve)=>{
            if (eve.clientY != 0 && eve.buttons == 1){
                new pos_control(eve.clientX - slider_bar.getBoundingClientRect().left, false);
            }
        }, true);

        slider_bar.addEventListener("touchmove", (eve)=>{
            new pos_control(eve.changedTouches[0].clientX - slider_bar.getBoundingClientRect().left, true);
        }, true)
        
        slider_bar.appendChild(slider);
        this.parent.appendChild(slider_bar);
        
        new pos_control((this.value * slider_bar.clientWidth) / this.max, true);
    }

}
let hr_range_slider = new range_slider(1, 180, 90, document.querySelector(".camera_controls .x_origin_rotation"), (cords)=>{
    sendCommand(`setXcam_rotor,${cords}`);
});

let vr_range_slide = new range_slider(1, 109, 54, document.querySelector(".camera_controls .y_origin_rotation"), (cords)=>{
    sendCommand(`setYcam_rotor,${cords}`);
});

function onClickLight(){
    let ele = document.getElementById("symbols_head_light");
    if (ele.style.backgroundColor){
        ele.style.backgroundColor = "";
        sendCommand("ledoff");
    }
    else{
        ele.style.backgroundColor = "orange";
        sendCommand("ledon");
    }
}

function onClickHorn(){
    let ele = document.getElementById("symbols_horn");
    if (ele.style.backgroundColor){
        ele.style.backgroundColor = "";
        sendCommand("hornOff");
    }
    else{
        ele.style.backgroundColor = "orange";
        sendCommand("hornOn");
    }
}

let defaultSpeed = 100;
let ret = 0;
document.querySelector("#speed input").onchange = (eve)=>{
    if (eve.target.value > 255){
        eve.target.value = 255;
    }
    else if (eve.target.value < 0){
        eve.target.value = 0;
    }
    sendCommand(`setSpeed,${eve.target.value}`);
}



// -------------- Camera work ----------------
let is_live = false;

function valueBar_interact(){
    let VB = document.querySelectorAll(".value_bar");
    for (let i = 0; i < VB.length; i++) {
        VB[i].querySelector("input").oninput = (eve)=>{VB[i].querySelector(".changeable_text").innerText=eve.target.value};
    }
}
valueBar_interact();

let camIPAddr = null;
function camSetup(){
    let xmlServer = new XMLHttpRequest();
    xmlServer.open("POST", "/giveCamIP");
    xmlServer.onload = ()=>{
        if (xmlServer.responseText.startsWith("http:")){
            camIPAddr = xmlServer.responseText;
            console.log(camIPAddr);
        }
        else{
            alert("cannot get camIP "+xmlServer.responseText);
        }
    }
    xmlServer.send("Data");
}
camSetup();

let is_full_screen = false;
function onFullScreenCam(){
    let display_win = document.getElementsByClassName("camera")[0];
    if (!is_full_screen){
        display_win.style.position = "absolute";
        display_win.style.top = "1px";
        display_win.style.left = "1px";
        display_win.style.width = "99%";
        display_win.style.height = "99%";
        is_full_screen = true;
    }
    else{
        display_win.style.position = "";
        display_win.style.top = "";
        display_win.style.left = "";
        display_win.style.width = "";
        display_win.style.height = "";
        is_full_screen = false;
    }
}

function enableCameraMode(){
    let ele_ = document.getElementsByClassName("camera_controls")[0];
    if (ele_.style.display == ""){
        document.getElementsByClassName("controls")[0].style.display = "none";
        ele_.style.display = "grid";
        hr_range_slider.createHorizontalRS();
        vr_range_slide.createVerticalRS();
    }
    else{
        document.getElementsByClassName("controls")[0].style.display = "";
        ele_.style.display = "";
    }
}

function create_View_Port(src_){
    let already_exist = document.querySelector(".display .camera .viewPhotage");
    if (already_exist){already_exist.remove()};
    let view_port = document.createElement("iframe");
    view_port.className = "viewPhotage";
    document.querySelector(".display .camera").appendChild(view_port);
    view_port.contentWindow.document.body.background = src_;
    view_port.contentWindow.document.body.style.backgroundSize = "100% 100%";
}
function onCameraCapture(){
    is_live = false;
    if (!camIPAddr){
        if (confirm("Camera IP is not loded yet. click ok to load.")){
            camSetup();
        }
        return;
    }
    create_View_Port(camIPAddr+"/image");
}

function onCameraLive(){
    is_live = true;
    if (!camIPAddr){
        if (confirm("Camera IP is not loded yet. click ok to load.")){
            camSetup();
        }
        return;
    }
    create_View_Port(camIPAddr+"/live");
}

function forCameraSettings(){
    let ele_ = document.getElementsByClassName("cam_settings_window")[0];
    if (ele_.style.display == 'block'){
        ele_.style.display = "none";
    }
    else{
        ele_.style.display = "block";
    }
}

function onApplySettings(){
    if (!camIPAddr){
        if (confirm("Camera IP is not loded yet. click ok to load.")){
            camSetup();
        }
        return;
    }
    document.querySelector(".cam_settings_window .main_settings .apply_changes").innerText = "Saving...";
    document.querySelector(".cam_settings_window .main_settings .apply_changes").style.cursor = "progress";
    document.querySelector(".cam_settings_window .main_settings .apply_changes").disabled = true;

    const resolution = document.querySelector(".cam_settings_window .main_settings #resolutions").value;
    const spacial_effect = document.querySelector(".cam_settings_window .main_settings #spacial_effect").value;
    const filter = document.querySelector(".cam_settings_window .main_settings #filters").value;
    const brightness = document.querySelector(".cam_settings_window .main_settings #brightness").value;
    const contrast = document.querySelector(".cam_settings_window .main_settings #contrast").value;
    const saturation = document.querySelector(".cam_settings_window .main_settings #saturation").value;
    const quality = 63-document.querySelector(".cam_settings_window .main_settings #quality").value;

    let to_send = `${resolution},${spacial_effect},${filter},${brightness},${contrast},${saturation},${quality},`;
    let xmlServer = new XMLHttpRequest();
    xmlServer.open("POST", camIPAddr+"/change_settings");
    xmlServer.onload = ()=>{
        console.log(xmlServer.responseText);
        document.querySelector(".cam_settings_window .main_settings .apply_changes").innerText = "Apply Changes ✅";
        document.querySelector(".cam_settings_window .main_settings .apply_changes").style.cursor = "";
        document.querySelector(".cam_settings_window .main_settings .apply_changes").disabled = false;
        if (xmlServer.responseText == "OK"){
            forCameraSettings();
        }
    }
    xmlServer.send(to_send);
    console.log(to_send);
}

let counter_flash = 1;
function on_flash(){
    let ele_ =  document.getElementById("symbols_cam_target");
    if (counter_flash == 1){
        counter_flash = 0;
        sendCommand("flashon");
        ele_.style.backgroundColor = "orange";
    }
    else{
        counter_flash = 1;
        sendCommand("flashoff");
        ele_.style.backgroundColor = "";
    }
}
