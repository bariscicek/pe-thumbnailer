function jsTrim(s){return s.replace(/(^\s+)|(\s+$)/g,"")}function Track(trackParm,objLink){if(objLink.innerText&&jsTrim(objLink.innerText))LinkText=objLink.innerText;else if(objLink.all&&objLink.all(0).alt)LinkText=objLink.all(0).alt;var strDomain=document.domain,s=trackParm.split("|"),strLinkArea,strLinkId,strPageRegion;if(trackParm.substring(0,1)=="|"){strPageRegion=s[1];strLinkArea=s[2];strLinkId=s[2]+s[3]}else{strLinkArea=s[0];strLinkId=s[0]+s[1]}if(typeof LinkText=="undefined"||!LinkText||LinkText=="")LinkText=strLinkId;ctUrl=objLink.href+"?LinkId="+strLinkId+"&LinkArea="+strLinkArea;if(typeof DCSext!="undefined"){DCSext.wt_strHeadlnLocale=detectedLocale;DCSext.wt_strCat=strLinkArea+"|"+detectedLocale;DCSext.wt_strUrl=window.location.href.toLowerCase();DCSext.wt_strArea=strPageRegion}return false}

function CopyCode(elemName){var obj=document.getElementById(elemName);window.clipboardData.setData("Text",obj.innerText)}



var b=window.navigator.appName;function DDFilterOn(e){try{ChangeDropDownImage(true);oMTPS_DD_PopUpDiv.style.top=oMTPS_DD_ImgArrow.height;if(oMTPS_DD_PopUpDiv.style.display!="inline"){oMTPS_DD_PopUpDiv.style.display="inline";if(document.addEventListener){document.addEventListener("click",TestForResizableAreaControl,false);window.addEventListener("resize",TestForResizableAreaControl,false)}else{document.attachEvent("onkeydown",HandleEvent,false);document.body.onclick=HideFromClick}}else setTimeout(HideThisMenu,0)}catch(err){throw err}e.cancelBubble=true;oMTPS_DD_Div.normalize();return}function RequestHideLanguagesMenu(){try{frameworksMenuTimer=window.setTimeout("HideLanguagesMenu()",250);return}catch(err){throw err}return}function ResetLanguagesMenuTimer(e){try{if(frameworksMenuTimer!=null){window.clearTimeout(frameworksMenuTimer);frameworksMenuTimer=null}}catch(err){throw err}e.cancelBubble=true;oMTPS_DD_PopUpDiv.normalize();return}function HideLanguagesMenu(){try{oMTPS_DD_PopUpDiv.style.display="none";frameworksMenuTimer=null;frameworksMenuVisible=false}catch(err){throw err}return}function HandleEvent(event){var nav=window.Event?true:false;if(nav)return NetscapeEventHandler(event);else return MicrosoftEventHandler()}function NetscapeEventHandler(e){if((e.which==13||e.which==27||e.which==84)&&e.target.type!="textarea"&&e.target.type!="submit"){setTimeout(HideThisMenu,0);document.removeEventListener("keydown",HandleEvent,false);e.cancelBubble=true;e.returnValue=false;e.cancel=true;e.preventDefault()}return}function MicrosoftEventHandler(){if((event.keyCode==13||event.keyCode==27||event.keyCode==84)&&event.srcElement.type!="textarea"&&event.srcElement.type!="submit"){event.returnValue=false;event.cancel=true;setTimeout(HideThisMenu,0);document.detachEvent("onkeydown",HandleEvent,false);event.cancelBubble=true}return}function HideFromClick(event){var objClicked=window.event.srcElement,objParent=objClicked.parentNode.parentNode;if(objParent.id!=oMTPS_DD_PopUpDiv.id&&objParent.id!=oMTPS_DD_Div.id){setTimeout(HideThisMenu,0);return}else{window.event.cancelBubble=true;return}}function TestForResizableAreaControl(e){try{if(e.type=="keydown"){if(e.which==84||e.which==13){setTimeout(HideThisMenu,0);document.removeEventListener("keydown",TestForResizableAreaControl,false);e.cancelBubble=true;return}}else if(e.type=="resize"){setTimeout(HideThisMenu,0);window.removeEventListener("resize",TestForResizableAreaControl,false);e.cancelBubble=true;return}if(e.type=="click")if(e.which==1||e.which==32){var eNode=e.target;if(eNode.id.length==0){setTimeout(HideThisMenu,0);document.removeEventListener("click",TestForResizableAreaControl,false);return false}else{var pNode=document.getElementById(eNode.id).parentNode.parentNode;if(eNode.id!=oMTPS_DD_PopUpDiv.id)if(pNode.id!=oMTPS_DD_PopUpDiv.id){setTimeout(HideThisMenu,0);document.removeEventListener("mousedown",TestForResizableAreaControl,false)}if(e.target.id==oMTPS_DD_ImgArrow.id){setTimeout(HideThisMenu,0);document.removeEventListener("mousedown",TestForResizableAreaControl,false)}if(e.target.id==oMTPS_DD_PanelLink.id){setTimeout(HideThisMenu,0);document.removeEventListener("mousedown",TestForResizableAreaControl,false)}}return}}catch(err){throw err}}function HideThisMenu(){oMTPS_DD_PopUpDiv.style.display="none";oMTPS_DD_ImgArrow.className="MTPS_DropDownImage LibC_arrow-off";document.body.onclick=null;return}function ChangeDropDownImage(boolOnOff){if(boolOnOff==true)oMTPS_DD_ImgArrow.className="MTPS_DropDownImage LibC_arrow-on";else oMTPS_DD_ImgArrow.className="MTPS_DropDownImage LibC_arrow-off"}function SetLangFilterAll(){FilterLanguageSpecificText("nu")}function SetLangFilter(CodeSnipID,isExistLangFilter){try{if(CodeSnipID!=null){ToggleLang(CodeSnipID);oServerBool=false}SetFilterText(isExistLangFilter);SetCodeSnippetLangFilterCookie()}catch(err){throw err}return}function ToggleLang(strThis){var langArray="";try{langArray=eval(strThis)}catch(err){return}try{for(i=0;i<langArray.length;i++){var thisCodeSnip=document.getElementById(langArray[i]);if(thisCodeSnip==null)continue;if(thisCodeSnip.style.display=="block"||thisCodeSnip.style.display=="")thisCodeSnip.style.display="none";else thisCodeSnip.style.display="block"}if(langArray.length>0){var thisCodeSnipElement=document.getElementById(langArray[0]);if(!thisCodeSnipElement==null){var thisCodeSnipParent=thisCodeSnipElement.parentNode.parentNode.parentNode.id;thisCodeSnipParent+="_h";var obj=document.getElementById(thisCodeSnipParent);if(obj!=null)if(obj.innerHTML.indexOf("minus.gif")>0)thisCodeSnipElement.parentNode.parentNode.style.display="block";else thisCodeSnipElement.parentNode.parentNode.style.display="none"}}}catch(err){throw err}return}function SetFilterText(isLangSpecTextExist){var spr=" : ";try{var intSetoMTPS_DD_PanelLinkText=0,objLastChecked=null,cbxColl=oMTPS_DD_PopUpDiv.getElementsByTagName("input");for(var i=0;i<cbxColl.length;i++)if(cbxColl[i].checked==true){intSetoMTPS_DD_PanelLinkText++;objLastChecked=cbxColl[i].value}if(intSetoMTPS_DD_PanelLinkText==0){oMTPS_DD_PanelLink.innerHTML=strConstLangFilterText+spr+strConstLangFilterNone;if(isLangSpecTextExist)FilterLanguageSpecificText("nu")}if(intSetoMTPS_DD_PanelLinkText==1){oMTPS_DD_PanelLink.innerHTML=strConstLangFilterText+spr+objLastChecked;if(isLangSpecTextExist){var devLang;switch(objLastChecked){case "Visual Basic":devLang="vb";break;case "C#":devLang="cs";break;case "c#":devLang="cs";break;case "C++":devLang="cpp";break;case "c++":devLang="cpp";break;case "JScript":devLang="cs";break;case "JSharp":devLang="cs";break;case "J#":devLang="cs";break;default:devLang="nu";break}FilterLanguageSpecificText(devLang)}}if(intSetoMTPS_DD_PanelLinkText>1){oMTPS_DD_PanelLink.innerHTML=strConstLangFilterText+spr+strConstLangFilterMulti;if(isLangSpecTextExist)FilterLanguageSpecificText("nu")}if(intSetoMTPS_DD_PanelLinkText==oMTPS_DD_PopUpDiv.childNodes.length){oMTPS_DD_PanelLink.innerHTML=strConstLangFilterText+spr+strConstLangFilterAll;if(isLangSpecTextExist)FilterLanguageSpecificText("nu")}intSetoMTPS_DD_PanelLinkText=0;objLastChecked=null}catch(err){throw err}return}function FilterLanguageSpecificText(devLang){var isIE=navigator.userAgent.toLowerCase().indexOf("msie")!=-1,oStyle;if(isIE){oStyle=document.styleSheets["LanguageSpecificTextStyle"];if(oStyle!=null){oStyle.removeRule(0);oStyle.addRule("."+devLang,"display:inline",0)}}else{oStyle=document.styleSheets;for(var i=0;i<oStyle.length;i++)if(oStyle[i].ownerNode.id=="LanguageSpecificTextStyle"){oStyle[i].deleteRule(0);oStyle[i].insertRule("."+devLang+"{display:inline}",0);break}}}function CheckDropDownClientCookie(isExistLangText){var cbxColl=oMTPS_DD_PopUpDiv.getElementsByTagName("input");for(var i=0;i<cbxColl.length;i++)if(cbxColl[i].checked!=true){var tStr=cbxColl[i].getAttribute("arrayValue");SetLangFilter(tStr,isExistLangText)}return}function SetCodeSnippetLangFilterCookie(){var daysToExpire=60,cookieDate=new Date;cookieDate.setTime(cookieDate.getTime()+daysToExpire*24*360000);expires="expires="+cookieDate.toGMTString();if(window.navigator.cookieEnabled==true){var cbxColl=oMTPS_DD_PopUpDiv.getElementsByTagName("input");for(i=0;i<cbxColl.length;i++)document.cookie=cbxColl[i].id+"_"+i+"="+cbxColl[i].checked+"; "+expires}return}

