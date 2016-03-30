<%@ Page Title="Map" Language="C#" MasterPageFile="~/Site.Master" AutoEventWireup="true" CodeBehind="Map.aspx.cs" Inherits="LightningWebApp.Map" EnableEventValidation="false"%>
<%@ register assembly="GMaps" namespace="Subgurim.Controles" tagprefix="cc1" %>

<asp:Content ID="BodyContent" ContentPlaceHolderID="MainContent" runat="server">
    <h2><%: Title %></h2>
    <cc1:GMap ID="GMap1" runat="server" Width="520px" />
    <br />
    <br />
    <div>
        <asp:Label ID="Label1" runat="server" Text="Device ID"></asp:Label>
        <asp:TextBox ID="txtDevice" runat="server" MaxLength="2" Width="40px"></asp:TextBox> &nbsp
        <asp:Button ID="btnApplyFilter" runat="server" OnClick="btnApplyFilter_Click" Text="Search" />
    </div>
    <br />
    <asp:GridView ID="gridLightningData" runat="server" CellPadding="4" ForeColor="#333333" GridLines="None" ShowFooter="True" Width="520px" OnRowDataBound="gridLightningData_RowDataBound" OnSelectedIndexChanged="OnSelectedIndexChanged">
        <AlternatingRowStyle BackColor="White" ForeColor="#284775" />
        <EditRowStyle BackColor="#999999" />
        <FooterStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
        <HeaderStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
        <PagerStyle BackColor="#284775" ForeColor="White" HorizontalAlign="Center" />
        <RowStyle BackColor="#F7F6F3" ForeColor="#333333" />
        <SelectedRowStyle BackColor="#E2DED6" Font-Bold="True" ForeColor="#333333" />
        <SortedAscendingCellStyle BackColor="#E9E7E2" />
        <SortedAscendingHeaderStyle BackColor="#506C8C" />
        <SortedDescendingCellStyle BackColor="#FFFDF8" />
        <SortedDescendingHeaderStyle BackColor="#6F8DAE" />
    </asp:GridView>
    <br />  <br />
    <font size="1"> Date and Time are in UTC. </font>
</asp:Content>