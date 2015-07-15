var g_scaJSONObject;
var g_rootPkg = {
        ssca_PkgName : "ROOT",
        pkg : 0,
        childPkgs : [ ]
};

//==============================================================================
///      \function loadFile 
///      \brief    Loads the JSON file 
//==============================================================================
function loadFile() {
  var input, file, fr;
  if (typeof window.FileReader !== 'function') {
    alert("The file API isn't supported on this browser yet.");
    return;
  }
  input = document.getElementById('fileinput');
  if (!input) {
    alert("Um, couldn't find the fileinput element.");
  }
  else if (!input.files) {
    alert("This browser doesn't seem to support the `files` property of file inputs.");
  }
  else if (!input.files[0]) {
    alert("Please select a file before clicking 'Load'");
  }
  else {
    file = input.files[0];
    fr = new FileReader();
    fr.onload = receivedText;
    fr.readAsText(file);
  }
  function receivedText(e) {
    lines = e.target.result;
    g_scaJSONObject = JSON.parse(lines); 
    main();
  }
}

//==============================================================================
//      \function baseName 
//      \brief    Given the absolute path returns the base name
//==============================================================================
function baseName(str)
{
   var base = new String(str).substring(str.lastIndexOf('/') + 1); 
    if(base.lastIndexOf(".") != -1)       
        base = base.substring(0, base.lastIndexOf("."));
   return base;
}

//==============================================================================
//      \function dirName 
//      \brief    Given the absolute path returns the dir name
//==============================================================================
function dirName(path) {
  return path.replace(/\\/g,'/').replace(/\/[^\/]*$/, '');
}

//==============================================================================
//      \function getPackage 
//      \brief    Given the package name returns the actual package
//==============================================================================
function getPackage(package_name) {
  for(i=0; i<g_scaJSONObject.pkg.length; i++) {
    if( g_scaJSONObject.pkg[i].ssca_PkgName == package_name) {
      return  g_scaJSONObject.pkg[i];
    }
  }
  return undefined;
}

//==============================================================================
//      \function numOfDirLevels 
//      \brief    Count number of '/' in the given path
//==============================================================================
function numOfDirLevels(str) {
   var base = new String(str);
   var ret_val = 0;
   for(i=0; i<base.length; i++) {
     if(base.charAt(i) == '/') {
       ret_val++;
     }
   }
   return ret_val;
}

//==============================================================================
//      \function buildHierarchyPkgTree 
//      \brief    Builds the hierarchy of packages 
//==============================================================================
function buildHierarchyPkgTree() {
  var map = {};
  var max_level = -1;
  var i=0;
  // First create a map based on level numbers
  for(i=0; i<g_scaJSONObject.pkg.length; i++) {
    var level_no = numOfDirLevels(g_scaJSONObject.pkg[i].ssca_PkgName);
    if(level_no in map) {
      map[level_no].push(g_scaJSONObject.pkg[i].ssca_PkgName);
    } else {
      map[level_no] = [ g_scaJSONObject.pkg[i].ssca_PkgName ];
    }
    if(max_level < level_no) {
      max_level = level_no;
    }
  }
  var parent_list = [ g_rootPkg ]; 
  for(var level_no=1; level_no<=max_level; level_no++) {
    if(!(level_no in map)) {
      continue;
    }
    for(var j=0; j<map[level_no].length; j++) {
      // A new package
      var child_pkg = { ssca_PkgName : (map[level_no])[j], 
                        pkg : "", 
                        childPkgs : [] };
      var found_match = 0;
      for(var k=parent_list.length-1; k>=0; k--) {
        if(dirName(child_pkg.ssca_PkgName) == parent_list[k].ssca_PkgName) {
          // Found the parent
          child_pkg.pkg = getPackage(child_pkg.ssca_PkgName);
          parent_list[k].childPkgs.push(child_pkg);
          parent_list.push(child_pkg);
          found_match = 1;
          break;
        }
      } 
      if(found_match == 0) {
        // Create a dummy package by that name dirname(child_pkg)
        var parent_pkg = { ssca_PkgName : dirName(child_pkg.ssca_PkgName), 
                                          pkg : "", 
                                          childPkgs : [] };
        parent_pkg.childPkgs.push(child_pkg);
        for(var l=parent_list.length-1; l>=0; l--) {
          if(dirName(parent_pkg.ssca_PkgName) == parent_list[l].ssca_PkgName) {
            parent_list[l].childPkgs.push(parent_pkg);
          }
        }
        parent_list.push(parent_pkg);
        parent_list.push(child_pkg);
      }
    }
  }
}

//==============================================================================
//      \function  showSourceHierarchy 
//      \brief     Given a package tree creates a jsTree recursively
//==============================================================================
function showSourceHierarchy(parent_package) {
  var ret_var = "<ul>";
  ret_var += "<li class=\"jstree-open\" \" id=\"";
  ret_var += parent_package.ssca_PkgName;
  ret_var += "\">"; 
  ret_var += parent_package.ssca_PkgName;
  for(var i=0; i<parent_package.childPkgs.length; i++) {
    var child_package = parent_package.childPkgs[i];
    ret_var += showSourceHierarchy(child_package);
  }
  ret_var += "</li></ul>";
  return ret_var;
}

//==============================================================================
//      \function main 
//      \brief    The main function executed after loading the JSON file
//==============================================================================
function main() {
  // First build the hierarachy tree
  buildHierarchyPkgTree();
  // Load the tree
  var ret_var = showSourceHierarchy(g_rootPkg);
  $('#nodeContainer').html(ret_var);
  executeTreejQuery();
}

//==============================================================================
//      \function executeTreejQuery
//      \brief    Executes all the jQuery calls
//==============================================================================
function executeTreejQuery() {
  // Clicking on the node must open the table
  $('#nodeContainer').on('changed.jstree', function (e, data) {
    var i, j, r = [];
    for(i = 0, j = data.selected.length; i < j; i++) {
      r.push(data.instance.get_node(data.selected[i]).text);
    }
    var package_name = r.join(', ');
    showPackageTable(package_name);
  }).jstree();
}

//==============================================================================
//      \function executeTabjQuery
//      \brief    Executes all the jQuery tabs
//==============================================================================
function executeTabjQuery() {
  // Tab changes
  jQuery('.tabs .tab-links a').on('click', function(e)  {
    var currentAttrValue = jQuery(this).attr('href');
    // Show/Hide Tabs

    jQuery('.tabs ' + currentAttrValue).show().siblings().hide();

    // Change/remove current tab to active
    jQuery(this).parent('li').addClass('active').siblings().removeClass('active');

    e.preventDefault();
  });
}

//==============================================================================
//      \function executeQToolTipQuery
//      \brief    Executes all the jQuery tabs
//==============================================================================
function executeQToolTipQuery() {
  $(".NameTooltip").hover(function () {
    $(this).append('<div class="tooltip"><p>Method/Function name</p></div>');
  }, function () {
    $("div.tooltip").remove();
  });

  $(".CycXTooltip").hover(function () {
    $(this).append('<div class="tooltip"><p>Cyclomatic complexity is a metric that tells how many different branches are possible inside a given function. This in turn tells the number of test cases needed to cover all the different paths possible inside the function</p> <p> Recomended upper limit : 20 </p></div>');
  }, function () {
    $("div.tooltip").remove();
  });

  $(".TimeXTooltip").hover(function () {
    $(this).append('<div class="tooltip"><p>Time complexity O(n)</p> <p> Recomended upper limit :  3 </p></div>');
  }, function () {
    $("div.tooltip").remove();
  });

  $(".MaxNTooltip").hover(function () {
    $(this).append('<div class="tooltip"><p>Maximum level of nexting in the function</p> <p> Recomended upper limit : </p> </div>');
  }, function () {
    $("div.tooltip").remove();
  });

  $(".NumLinesTooltip").hover(function () {
    $(this).append('<div class="tooltip"><p>Number of lines in the function. The </p> <p> Recomended upper limit : </p> </div>');
  }, function () {
    $("div.tooltip").remove();
  });

  $(".ParamSizeTooltip").hover(function () {
    $(this).append('<div class="tooltip"><p>This metric represents the number of fields in a method. Although a large number of fields is not necessarily an indication of bad code, it does suggest the possibility of grouping fields together and extracting classes</p> <p> Recomended upper limit : </p> </div>');
  }, function () {
    $("div.tooltip").remove();
  });
}

//==============================================================================
//      \function 
//      \brief    
//==============================================================================
function getSummaryMetrics(package_name) {
  return "";
}

//==============================================================================
//      \function 
//      \brief    
//==============================================================================
function getFilesMetrics(package_name) {
  return "";
}

//==============================================================================
//      \function 
//      \brief    
//==============================================================================
function getHeader1() {
  var ret_var = "<thead><tr>";
  ret_var += "<th> Name <span id=\"NameTooltip\" class=\"question NameTooltip\">?</span> </th>";
  ret_var += "<th> Cyclomatic Cmplx <span id=\"CycXTooltip\" class=\"question CycXTooltip\">?</span> </th>";
  ret_var += "<th> Time Cmplx <span id=\"TimeXTooltip\" class=\"question TimeXTooltip\">?</span> </th>";
  ret_var += "<th> Maximum nesting <span id=\"MaxNTooltip\" class=\"question MaxNTooltip\">?</span> </th>";
  ret_var += "<th> Num lines <span id=\"NumLinesTooltip\" class=\"question NumLinesTooltip\">?</span> </th>";
  ret_var += "<th> Parameter size <span id=\"ParamSizeTooltip\" class=\"question ParamSizeTooltip\">?</span> </th>";
  ret_var += "</tr></thead>";
  return ret_var;
}

//==============================================================================
//      \function 
//      \brief    
//==============================================================================
function getFunctionMetrics(cur_pkg) {
  var ret_var = "<table id=\"MetricsTable1\" class=\"tablesorter\" style=\"width:100%\">";
  ret_var += getHeader1();
  ret_var += "<tbody>";
  var map = {};
  for(i=0; i<cur_pkg.ssca_sFile.length; i++) {
    var cur_file = cur_pkg.ssca_sFile[i];
    for(j=0; j<cur_file.ssca_funct.length; j++) {
      var cur_func = cur_file.ssca_funct[j];
      if(!(cur_func.ssca_FunctionName in map)) {
        ret_var += "<tr>";
        ret_var += "<td>" + cur_func.ssca_FunctionName + "</td>";
        ret_var += "<td>" + cur_func.ssca_cyc + "</td>";
        ret_var += "<td>" + cur_func.ssca_cmp + "</td>";
        ret_var += "<td>" + cur_func.ssca_mn + "</td>";
        ret_var += "<td>" + cur_func.ssca_nl + "</td>";
        ret_var += "<td>" + cur_func.ssca_ps + "</td>";
        ret_var += "</tr>";
        map[cur_func.ssca_FunctionName] = 1;
      }
    }
  }
  ret_var += "</tbody></table>";
  return ret_var;
}

//==============================================================================
//      \function 
//      \brief    
//==============================================================================
function getMethodMetrics(cur_pkg) {
  var ret_var = "<table id=\"MetricsTable2\" class=\"tablesorter\" style=\"width:100%\">";
  ret_var += getHeader1();
  ret_var += "<tbody>";
  var map = {};
  for(i=0; i<cur_pkg.ssca_hFile.length; i++) {
    var cur_file = cur_pkg.ssca_hFile[i];
    for(j=0; j<cur_file.ssca_cls.length; j++) {
      var cur_class = cur_file.ssca_cls[j];
      for(k=0; k< cur_class.ssca_method.length; k++) {
        var cur_func = cur_class.ssca_method[k];
        if(!(cur_func.ssca_i in map)) {
          ret_var += "<tr>";
          ret_var += "<td>" + cur_func.ssca_MethodName + "</td>";
          ret_var += "<td>" + cur_func.ssca_cyc + "</td>";
          ret_var += "<td>" + cur_func.ssca_cmp + "</td>";
          ret_var += "<td>" + cur_func.ssca_mn + "</td>";
          ret_var += "<td>" + cur_func.ssca_nl + "</td>";
          ret_var += "<td>" + cur_func.ssca_ps + "</td>";
          ret_var += "</tr>";
          map[cur_func.ssca_i] = 1;
        }
      }
    }
  }
  ret_var += "</tbody></table>";
  return ret_var;
}

//==============================================================================
//      \function 
//      \brief    
//==============================================================================
function getClassMetrics(cur_pkg) {
  return "";
}

//==============================================================================
//      \function 
//      \brief    
//==============================================================================
function showPackageTable(package_name)
{
    if(package_name == "ROOT") {
      return;
    }    

    var var1 = "";
    var1 += "<h3>" + package_name + "</h3>";
    $('#packageName').html(var1);

    var ret_var = "";
    var package_obj     = getPackage(package_name); 

    ret_var += getSummaryMetrics(package_name);
    $("#SummaryTab").html(ret_var);
   
    ret_var = "";
    ret_var += getFilesMetrics(package_name);
    $("#FileMetricsTab").html(ret_var);

    ret_var = "";
    ret_var += getFunctionMetrics(package_obj);
    $("#FunctionMetricsTab").html(ret_var);

    ret_var = "";
    ret_var += getMethodMetrics(package_obj);
    $("#MethodMetricsTab").html(ret_var);

    ret_var = "";
    ret_var += getClassMetrics(package_name);
    $("#ClassMetricsTab").html(ret_var);
   
    $("#MetricsTable1").tablesorter(); 
    $("#MetricsTable2").tablesorter(); 
    executeTabjQuery();
    executeQToolTipQuery();
}

