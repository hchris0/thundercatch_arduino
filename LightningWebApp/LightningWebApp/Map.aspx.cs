using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Auth;
using Microsoft.WindowsAzure.Storage.Table;
using Subgurim.Controles;
using System.Drawing;

namespace TestSiteExample
{
    public partial class Map : System.Web.UI.Page
    {

        CloudStorageAccount account = new CloudStorageAccount(
new StorageCredentials([Storage account name],
[Storage account key]), true); 

        public class Lightnings : TableEntity
        {
            public Lightnings() { }
            public string Distance { get; set; }
            public string Event { get; set; }
            public string Device { get; set; }
        } 

        protected void Page_Load(object sender, EventArgs e)
        {
            if (!this.IsPostBack) {
                initGMap();
                fillGrid(readTable(false));
            }
        }

        private void getDateFilter()
        {
            DateTime dt = DateTime.Now;
            dt = dt.AddDays(-2);
            string day = dt.Day.ToString("D2");
            string month = dt.Month.ToString("D2");
            string year = dt.Year.ToString("D2");
            string dateFilter = year + "-" + month + "-" + day + "T00:00:00Z";
        }

        public void fillGrid(IEnumerable<Lightnings> data)
        {

            DataTable dt = new DataTable();
            DataRow dr = null;
            dt.Columns.Add(new DataColumn("Device ID", typeof(string)));
            dt.Columns.Add(new DataColumn("Distance (km)", typeof(string)));
            dt.Columns.Add(new DataColumn("Date", typeof(string)));
            dt.Columns.Add(new DataColumn("Time", typeof(string)));

            if (!data.Any<Lightnings>()) {
                dr = dt.NewRow();
                dr["Device ID"] = string.Empty;
                dr["Distance (km)"] = string.Empty;
                dr["Date"] = string.Empty;
                dr["Time"] = string.Empty;
                dt.Rows.Add(dr);
            } else {
                // Print the fields for each customer. 
                foreach (Lightnings entity in data) {
                    DateTimeOffset dto = new DateTimeOffset();
                    dto = entity.Timestamp;
                    dr = dt.NewRow();
                    dr["Device ID"] = entity.Device;
                    dr["Distance (km)"] = entity.Distance;
                    dr["Date"] = dto.Day.ToString("D2") + "-" + dto.Month.ToString("D2") + "-" + dto.Year.ToString();
                    dr["Time"] = dto.Hour.ToString("D2") + ":" + dto.Minute.ToString("D2") + ":" + dto.Second.ToString("D2");
                    dt.Rows.Add(dr);
                }
            }
            //Store the DataTable in ViewState
            ViewState["CurrentTable"] = dt;

            gridLightningData.DataSource = dt;
            gridLightningData.DataBind();
        }

        public IEnumerable<Lightnings> readTable(bool readAllTable)
        {
            // Create the table client 
            CloudTableClient tableClient = account.CreateCloudTableClient();
            // Create the CloudTable object that represents the table
            CloudTable table = tableClient.GetTableReference([Storage table name]);
            // Query filter
            string filter = "(PartitionKey eq //PartitionKey//)";
            // Construct the query operation 
            TableQuery<Lightnings> query = new TableQuery<Lightnings>().Where(filter);
            // Get data from table
            IEnumerable<Lightnings> data = table.ExecuteQuery(query);
            var sortedData = data.OrderByDescending(c => c.Timestamp);
            return sortedData;
        }

        private string createFilter(bool queryAll)
        {
            string filter = "(PartitionKey eq //PartitionKey//) and (Event eq '3')";
            if (queryAll) {
                return filter;
            }
            string deviceFilter = "";
            string eventFilter = "";
            string dateFrom = "";
            string dateTo = "";
            string dateFilter = "";
            bool from = false;
            bool to = false;
            if (txtDevice.Text != "") {
                deviceFilter = " and (Device eq '" + txtDevice.Text + "')";
            }

            if (from & to) {
                dateFilter = " and (" + dateFrom + " and " + dateTo + ")";
            } else if (from) {
                dateFilter = " and (" + dateFrom + ")";
            } else if (to) {
                dateFilter = " and (" + dateTo + ")";
            }
            filter += deviceFilter + eventFilter + dateFilter;
            return filter;
        }

        protected void btnApplyFilter_Click(object sender, EventArgs e)
        {
            fillGrid(readTable(false));
        }

        protected void gridLightningData_RowDataBound(object sender, GridViewRowEventArgs e)
        {
            if (e.Row.RowType == DataControlRowType.DataRow) {
                e.Row.Attributes["onclick"] = Page.ClientScript.GetPostBackClientHyperlink(gridLightningData, "Select$" + e.Row.RowIndex);
                e.Row.Attributes["style"] = "cursor:pointer";
            }
        }
              
        protected void OnSelectedIndexChanged(object sender, EventArgs e)
        { 
            string devid = gridLightningData.SelectedRow.Cells[0].Text;
            string distance = gridLightningData.SelectedRow.Cells[1].Text;
            double lat, lngt;
            switch (devid) {
                case "1":
                    lat = 62.92286;
                    lngt = 26.26183;
                    break;
                case "2":
                    lat = 51.96537;
                    lngt = 17.95107;
                    break;
                case "3":
                    lat = 52.37020;
                    lngt = 0.99579;
                    break;
                default:
                    lat = 42.06480;
                    lngt = 13.93973;
                    break;
            }
            drawCircleInMap(lat, lngt, Convert.ToInt32(distance));
        }

        private void drawCircleInMap(double lat, double lng, int radius)
        {
            GMap1.resetPolylines();
            var d2r = Math.PI / 180;   // degrees to radians
            var r2d = 180 / Math.PI;   // radians to degrees
            var earthsradius = 6378.35; // 6378.35 is the radius of the earth in km
            var points = 50;
            double rlat = ((double)radius / earthsradius) * r2d;
            double rlng = rlat / Math.Cos(lat * d2r);
            List<GLatLng> extp = new List<GLatLng>();
            for (var i = 0; i < points + 1; i++) {
                double theta = Math.PI * (i / (double)(points / 2));
                double ex = lng + (rlng * Math.Cos(theta));
                double ey = lat + (rlat * Math.Sin(theta));
                extp.Add(new GLatLng(ey, ex));
            }
            GPolyline linea = new GPolyline(extp, "FF0000", 1, 0.8);
            GMap1.Add(linea);
            GMap1.setCenter(new GLatLng(lat, lng), 9);
        }

        private void initGMap()
        {
            GMap1.Language = "gr";
            GMap1.Height = 400;
            GMap1.Width = 520;
            GMap1.setCenter(new GLatLng(37.9776341, 23.7335321), 9);
            GMap1.enableHookMouseWheelToZoom = true;
            GMap1.mapType = GMapType.GTypes.Hybrid;
            GControl control = new GControl(GControl.preBuilt.LargeMapControl);
            GControl control2 = new GControl(GControl.preBuilt.MenuMapTypeControl, new GControlPosition(GControlPosition.position.Top_Right));

            GMap1.Add(control);
            GMap1.Add(control2);
        }
        
    }
}