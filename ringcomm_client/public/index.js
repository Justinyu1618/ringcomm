const pages = ["Home", "Text", "Camera", "Slides"];

let currentPage = "Home";

function changePage(pageName) {
  console.log("change page: ", pageName);
  pages.forEach((pg) => {
    if (pg === pageName) {
      $("#" + pg).css("display", "flex");
      currentPage = pageName;
    } else {
      $("#" + pg).css("display", "none");
    }
  });
}
