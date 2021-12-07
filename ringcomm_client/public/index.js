const pages = ["Home", "Text", "Camera", "Slides"];

let currentPage = "Home";
let currentPageN = 0;

function changePage(pageName) {
  console.log("change page: ", pageName);
  pages.forEach((pg) => {
    if (pg === pageName) {
      $("#" + pg).css("display", "flex");
      currentPage = pageName;
      currentPageN = pages.indexOf(pageName);
    } else {
      $("#" + pg).css("display", "none");
    }
  });
}
